#include "stdafx.h"
#include <fstream>
#include <filesystem>
#include "WaveManager.h"
#include "WTextureLoader.h"
#include "WMeshLoader.h"

namespace WaveE
{
	WAVEE_SINGLETON_CPP(WaveManager);

	WaveManager::WaveManager(const WaveEDescriptor& rDescriptor)
		: m_gameCamera{ {0, 8, 0}, 0, 15, 45, static_cast<float>(rDescriptor.width) / rDescriptor.height, 0.1f, 100.f }
	{
		ms_pInstance = this;

		m_gameCameraControls = rDescriptor.cameraControls;

		m_height = rDescriptor.height;
		m_width = rDescriptor.width;
		m_targetFrameTime = 1.f / rDescriptor.targetFrameRate;

		InitWindow(rDescriptor);
		InitDX12(rDescriptor);

		// Init all singletons
		WTextureLoader::Init();
		WResourceManager::Init();
		WMeshLoader::Init();
		WInput::Init();

		m_cbvSrvUavHeap.Init(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE, m_descriptorHeapCountCBV_SRV_UAV);
		m_rtvHeap.Init(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, D3D12_DESCRIPTOR_HEAP_FLAG_NONE, m_descriptorHeapCountRTV);
		m_dsvHeap.Init(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, D3D12_DESCRIPTOR_HEAP_FLAG_NONE, m_descriptorHeapCountDSV);
		m_samplerHeap.Init(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE, m_descriptorHeapCountSampler);

		// Create render target views for backbuffers
		for (UINT i = 0; i < m_frameCount; i++)
		{
			m_backBufferAllocations[i] = m_rtvHeap.Allocate(1);

			D3D12_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
			renderTargetViewDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
			renderTargetViewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

			m_pDevice->CreateRenderTargetView(m_pBackBuffers[i].Get(), nullptr, m_rtvHeap.GetCPUHandle(m_backBufferAllocations[i]));
		}

		m_uploadManager.Init(m_bigUploadBufferSize, m_bigUploadBufferCount, m_smallUploadBufferSize, m_smallUploadBufferCount);

		CreateDefaultRootSigniture();
		CreateSlotHLSLIFile();

		// Load Resources
		WResourceManager::Instance()->LoadShadersFromDirectory(GetShaderDirectory());
		WResourceManager::Instance()->LoadTexturesFromDirectory(GetTextureDirectory());
		WResourceManager::Instance()->LoadMeshesFromDirectory(GetMeshDirectory());

		// Default samplers
		WSamplerDescriptor samplerDescriptors[4] = {
			WSamplerDescriptor{ WSamplerDescriptor::Filter::Point, WSamplerDescriptor::AddressMode::Wrap },
			WSamplerDescriptor{ WSamplerDescriptor::Filter::Linear, WSamplerDescriptor::AddressMode::Wrap },
			WSamplerDescriptor{ WSamplerDescriptor::Filter::Point, WSamplerDescriptor::AddressMode::Clamp },
			WSamplerDescriptor{ WSamplerDescriptor::Filter::Linear, WSamplerDescriptor::AddressMode::Clamp }
		};

		m_defaultSamplers = WResourceManager::Instance()->CreateResourceBlock(samplerDescriptors, 4);

		// Define the default vertex input layout.
		m_defaultInputElements[0] = D3D12_INPUT_ELEMENT_DESC{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
		m_defaultInputElements[1] = D3D12_INPUT_ELEMENT_DESC{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
		m_defaultInputElements[2] = D3D12_INPUT_ELEMENT_DESC{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

		m_defaultInputLayout = { m_defaultInputElements, _countof(m_defaultInputElements) };
		
		// Default pipeline
		WPipelineDescriptor pipelineDescriptor;
		pipelineDescriptor.pVertexShader = WResourceManager::Instance()->GetShader(m_defaultVertexShaderName);
		pipelineDescriptor.pPixelShader = WResourceManager::Instance()->GetShader(m_defaultPixelShaderName);
		pipelineDescriptor.dsvFormat = m_defaultDepthType;
		m_defaultPipeline3D = WResourceManager::Instance()->CreateResource(pipelineDescriptor);

		// Default depth
		WTextureDescriptor depthTextureDescriptor;
		
		depthTextureDescriptor.format = WTextureDescriptor::DepthFloat;
		depthTextureDescriptor.usage = WTextureDescriptor::RenderTarget;
		depthTextureDescriptor.startAsShaderResource = false;
		depthTextureDescriptor.width = GetWidth();
		depthTextureDescriptor.height = GetHeight();
		m_defaultDepthTexture = WResourceManager::Instance()->CreateResource(depthTextureDescriptor);

		// Frame buffers
		WBufferDescriptor cameraAndLightBufferDescriptors[2] = {};
		cameraAndLightBufferDescriptors[0].sizeBytes = sizeof(CameraBuffer);
		cameraAndLightBufferDescriptors[1].sizeBytes = sizeof(LightBuffer);

		m_cameraAndLightBuffers = WResourceManager::Instance()->CreateResourceBlock(cameraAndLightBufferDescriptors, 2);

		// Time
		InitTime();

	}

	void WaveManager::Init(const WaveEDescriptor& rDescriptor)
	{
		if (!ms_pInstance)
		{
			new WaveManager{ rDescriptor };
		}
	}

	void WaveManager::EndInit()
	{
		// Excicute command list for setup
		HRESULT hr = Instance()->m_pCommandList->Close();
		WAVEE_ASSERT_MESSAGE(SUCCEEDED(hr), "Failed to close command list!");

		ID3D12CommandList* ppCommandLists[] = { Instance()->m_pCommandList.Get() };
		Instance()->m_pCommandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

		// Schedule a Signal command in the queue.
		const UINT64 currentFenceValue = Instance()->m_fenceValues[Instance()->m_frameIndex];
		hr = Instance()->m_pCommandQueue->Signal(Instance()->m_pFence.Get(), currentFenceValue);

		// Update the frame index.
		Instance()->m_frameIndex = Instance()->m_pSwapChain->GetCurrentBackBufferIndex();

		// If the next frame is not ready to be rendered yet, wait until it is ready.
		if (Instance()->m_pFence->GetCompletedValue() < Instance()->m_fenceValues[Instance()->m_frameIndex])
		{
			hr = Instance()->m_pFence->SetEventOnCompletion(Instance()->m_fenceValues[Instance()->m_frameIndex], Instance()->m_fenceEvent);
			WAVEE_ASSERT_MESSAGE(SUCCEEDED(hr), "Failed to set on completion event!");
			WaitForSingleObjectEx(Instance()->m_fenceEvent, INFINITE, FALSE);
		}

		// Set the fence value for the next frame.
		Instance()->m_fenceValues[Instance()->m_frameIndex] = currentFenceValue + 1;

		Instance()->m_uploadManager.EndFrame();
	}

	void WaveManager::Uninit()
	{
		delete ms_pInstance;
		ms_pInstance = nullptr;
	}

	WaveManager::~WaveManager()
	{
		// UnInit all singletons
		WTextureLoader::Uninit();
		WResourceManager::Uninit();
		WMeshLoader::Uninit();
		WInput::Uninit();
	}

	void WaveManager::InitWindow(const WaveEDescriptor& rDescriptor)
	{
		TCHAR projectDir[MAX_PATH];
		GetCurrentDirectory(MAX_PATH, projectDir);
		std::wstring projectDirWString{ projectDir };
		projectDirWString += L"\\..\\WaveE.ico";

		const wchar_t* className = L"WaveE";
		WNDCLASSEX windowClass;
		windowClass.cbSize = sizeof(WNDCLASSEX);
		windowClass.style = CS_HREDRAW | CS_VREDRAW;
		windowClass.lpfnWndProc = WindowProc;
		windowClass.cbClsExtra = 0;
		windowClass.cbWndExtra = 0;
		windowClass.hInstance = m_hInstance;
		windowClass.hIcon = (HICON)LoadImage(m_hInstance, projectDirWString.c_str(), IMAGE_ICON, 0, 0, LR_LOADFROMFILE | LR_DEFAULTSIZE | LR_SHARED);//LoadIcon(m_hInstance, MAKEINTRESOURCE(IDI_ICON1));
		windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
		windowClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
		windowClass.lpszMenuName = nullptr;
		windowClass.lpszClassName = className;
		windowClass.hIconSm = LoadIcon(m_hInstance, IDI_APPLICATION);

		if (!RegisterClassEx(&windowClass))
		{
			DWORD error = GetLastError();

			WAVEE_ERROR_DWORD(error);
		}

		RECT windowRect = { 0, 0, static_cast<LONG>(rDescriptor.width), static_cast<LONG>(rDescriptor.height) };
		AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);

		// Create the window and store a handle to it.
		m_hwnd = CreateWindow(
			windowClass.lpszClassName,
			className,
			WS_OVERLAPPEDWINDOW,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			windowRect.right - windowRect.left,
			windowRect.bottom - windowRect.top,
			nullptr,        // We have no parent window.
			nullptr,        // We aren't using menus.
			m_hInstance,
			this);

		if (!m_hwnd)
		{
			DWORD error = GetLastError();

			WAVEE_ERROR_DWORD(error);
		}

		WAVEE_ASSERT_MESSAGE(m_hwnd != NULL, "Failed to create window!");

		SetWindowTextA(m_hwnd, rDescriptor.title);
		ShowWindow(m_hwnd, SW_SHOWNORMAL);
		UpdateWindow(m_hwnd);

		RAWINPUTDEVICE rid;
		rid.usUsagePage = HID_USAGE_PAGE_GENERIC;
		rid.usUsage = HID_USAGE_GENERIC_MOUSE;
		rid.dwFlags = RIDEV_INPUTSINK;
		rid.hwndTarget = m_hwnd;
		WAVEE_ASSERT_MESSAGE(RegisterRawInputDevices(&rid, 1, sizeof(rid)), "Failed to register raw input!");
	}

	void WaveManager::InitDX12(const WaveEDescriptor& rDescriptor)
	{
		UINT dxgiFactoryFlags = 0;

#if defined(_DEBUG)
		// Enable the debug layer (requires the Graphics Tools "optional feature").
		// NOTE: Enabling the debug layer after device creation will invalidate the active device.
		{
			ComPtr<ID3D12Debug> debugController;
			if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
			{
				debugController->EnableDebugLayer();

				// Enable additional debug layers.
				dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;

				ComPtr<ID3D12Debug1> debugController1;
				if (SUCCEEDED(debugController.As(&debugController1)))
				{
					debugController1->SetEnableGPUBasedValidation(TRUE);
				}

			}
			else
			{
				//WAVEE_ASSERT_MESSAGE(false, "Failed to get debug interface!");
			}
		}
#endif
		// Create factory
		ComPtr<IDXGIFactory4> factory;
		HRESULT hr = CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&factory));

		WAVEE_ASSERT_MESSAGE(SUCCEEDED(hr), "Failed to create factory!");

		ComPtr<IDXGIAdapter1> hardwareAdapter;
		GetHardwareAdapter(factory.Get(), &hardwareAdapter);

		// Create device
		hr = D3D12CreateDevice(
			hardwareAdapter.Get(),
			D3D_FEATURE_LEVEL_11_0,
			IID_PPV_ARGS(&m_pDevice)
		);

		WAVEE_ASSERT_MESSAGE(SUCCEEDED(hr), "Failed to create device!");

		// Describe and create the command queue.
		D3D12_COMMAND_QUEUE_DESC queueDesc = {};
		queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

		hr = m_pDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_pCommandQueue));
		
		WAVEE_ASSERT_MESSAGE(SUCCEEDED(hr), "Failed to create command queue!");

		// Describe and create the swap chain.
		DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
		swapChainDesc.BufferCount = m_frameCount;
		swapChainDesc.Width = m_width;
		swapChainDesc.Height = m_height;
		swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		swapChainDesc.SampleDesc.Count = 1;

		ComPtr<IDXGISwapChain1> swapChain;
		hr = factory->CreateSwapChainForHwnd(
			m_pCommandQueue.Get(),        // Swap chain needs the queue so that it can force a flush on it.
			m_hwnd,
			&swapChainDesc,
			nullptr,
			nullptr,
			&swapChain
		);

		WAVEE_ASSERT_MESSAGE(SUCCEEDED(hr), "Failed to create swap chain!");

		hr = swapChain.As(&m_pSwapChain);

		WAVEE_ASSERT_MESSAGE(SUCCEEDED(hr), "Failed to create swap chain!");

		m_frameIndex = m_pSwapChain->GetCurrentBackBufferIndex();

		// Create the command allocators and back buffers
		for (UINT i = 0; i < m_frameCount; i++)
		{
			hr = m_pDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_pCommandAllocators[i]));
			WAVEE_ASSERT_MESSAGE(SUCCEEDED(hr), "Failed to create command allocator!");

			hr = m_pSwapChain->GetBuffer(i, IID_PPV_ARGS(&m_pBackBuffers[i]));
			WAVEE_ASSERT_MESSAGE(SUCCEEDED(hr), "Failed to get back buffer!");

			m_backBufferStates[i] = STATE_PRESENT;
		}

		// Create the command list.
		hr = m_pDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_pCommandAllocators[m_frameIndex].Get(), nullptr, IID_PPV_ARGS(&m_pCommandList));
		WAVEE_ASSERT_MESSAGE(SUCCEEDED(hr), "Failed to create command list!");

		// Create synchronization objects
		{
			hr = m_pDevice->CreateFence(m_fenceValues[m_frameIndex], D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_pFence));
			WAVEE_ASSERT_MESSAGE(SUCCEEDED(hr), "Failed to create fence!");
			m_fenceValues[m_frameIndex]++;

			// Create an event handle to use for frame synchronization.
			m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
			WAVEE_ASSERT_MESSAGE(m_fenceEvent, "Failed to create fence event!");
		}
	}

	LRESULT CALLBACK WaveManager::WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		WaveManager* pWaveManager = reinterpret_cast<WaveManager*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

		switch (message)
		{
			case WM_CREATE:
			{
				// Save the pointer passed in to CreateWindow.
				LPCREATESTRUCT pCreateStruct = reinterpret_cast<LPCREATESTRUCT>(lParam);
				SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pCreateStruct->lpCreateParams));
				return 0;
			}
			case WM_DESTROY:
			{
				PostQuitMessage(0);
				return 0;
			}
			case WM_SIZE:
			{
				if (pWaveManager)
				{
					pWaveManager->m_width = LOWORD(lParam);
					pWaveManager->m_height = HIWORD(lParam);
					pWaveManager->m_hasWindowSizeChanged = true;
				}
				return 0;
			}
			case WM_INPUT:
			{
				if (pWaveManager)
				{
					pWaveManager->HandleRawInput(lParam);
				}
				return 0;
			}
			case WM_KEYDOWN: [[fallthrough]];
			case WM_KEYUP:
			{
				if (pWaveManager)
				{
					pWaveManager->HandleKeyEvent(wParam, message == WM_KEYDOWN);
				}
				return 0;
			}
			case WM_LBUTTONDOWN: [[fallthrough]];
			case WM_LBUTTONUP: [[fallthrough]];
			case WM_RBUTTONDOWN: [[fallthrough]];
			case WM_RBUTTONUP: [[fallthrough]];
			case WM_MBUTTONDOWN: [[fallthrough]];
			case WM_MBUTTONUP: [[fallthrough]];
			case WM_MOUSEMOVE: [[fallthrough]];
			{
				if (pWaveManager)
				{
					pWaveManager->HandleMouseMove(message, wParam, lParam);
				}
				return 0;
			}
		}

		// Handle any messages the switch statement didn't.
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	void WaveManager::HideCursor()
	{
		::ShowCursor(FALSE);
	}

	void WaveManager::ShowCursor()
	{
		::ShowCursor(TRUE);
	}

	void WaveManager::ConfineCursor()
	{
		RECT rect;
		GetClientRect(m_hwnd, &rect);
		MapWindowPoints(m_hwnd, nullptr, reinterpret_cast<POINT*>(&rect), 2);
		ClipCursor(&rect);
	}

	void WaveManager::ReleaseCursor()
	{
		ClipCursor(nullptr);
	}

	void WaveManager::HandleKeyEvent(WPARAM wParam, bool isDown)
	{
		if (wParam < 256)
		{
			m_currentKeyState.set(wParam, isDown);
		}
	}

	void WaveManager::HandleMouseMove(UINT message, WPARAM wParam, LPARAM lParam)
	{
		// Update mouse button states
		switch (message)
		{
		case WM_LBUTTONDOWN:
			m_currentMouseState.leftMouseButton = true;
			break;
		case WM_LBUTTONUP:
			m_currentMouseState.leftMouseButton = false;
			break;
		case WM_RBUTTONDOWN:
			m_currentMouseState.rightMouseButton = true;
			break;
		case WM_RBUTTONUP:
			m_currentMouseState.rightMouseButton = false;
			break;
		case WM_MBUTTONDOWN:
			m_currentMouseState.middleMouseButton = true;
			break;
		case WM_MBUTTONUP:
			m_currentMouseState.middleMouseButton = false;
			break;
		}
		
		// Update mouse position
		if (message == WM_MOUSEMOVE)
		{
			POINTS points = MAKEPOINTS(lParam);
			m_currentMouseState.mousePosition.x = points.x;
			m_currentMouseState.mousePosition.y = points.y;
		}
	}

	void WaveManager::HandleRawInput(LPARAM lParam)
	{
		UINT dwSize = 0;
		GetRawInputData(reinterpret_cast<HRAWINPUT>(lParam), RID_INPUT, nullptr, &dwSize, sizeof(RAWINPUTHEADER));
		std::vector<BYTE> lpb(dwSize);
		WAVEE_ASSERT_MESSAGE(GetRawInputData(reinterpret_cast<HRAWINPUT>(lParam), RID_INPUT, lpb.data(), &dwSize, sizeof(RAWINPUTHEADER)) == dwSize, "Failed to get raw input data!");
		
		RAWINPUT* raw = reinterpret_cast<RAWINPUT*>(lpb.data());
		if (raw->header.dwType == RIM_TYPEMOUSE)
		{
			m_currentMouseState.delta.x += static_cast<float>(raw->data.mouse.lLastX);
			m_currentMouseState.delta.y += static_cast<float>(raw->data.mouse.lLastY);
		}
	}

	bool WaveManager::UpdateWindowLoop()
	{
		MSG msg = {};

		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
			{
				return true;
			}

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		return false;
	}

	bool WaveManager::BeginFrame()
	{
		if (UpdateWindowLoop())
		{
			return false;
		}

		UpdateTime();

		// Mouse input and camera movement
		if (WInput::Instance()->WasMouseButtonPressed(MOUSE_RIGHT))
		{
			HideCursor();
			ConfineCursor();
		}
		else if (WInput::Instance()->WasMouseButtonReleased(MOUSE_RIGHT))
		{
			ShowCursor();
			ReleaseCursor();
		}
		if (WInput::Instance()->IsMouseButtonDown(MOUSE_RIGHT))
		{
			UpdateGameCamera();
		}

		HRESULT hr = m_pCommandAllocators[m_frameIndex]->Reset();
		WAVEE_ASSERT_MESSAGE(SUCCEEDED(hr), "Failed to reset command allocator!");

		hr = m_pCommandList->Reset(m_pCommandAllocators[m_frameIndex].Get(), nullptr);
		WAVEE_ASSERT_MESSAGE(SUCCEEDED(hr), "Failed to reset command list!");

		SetDefaultRootSigniture();

		ID3D12DescriptorHeap* ppHeaps[] = { m_cbvSrvUavHeap.GetHeap(), m_samplerHeap.GetHeap() };
		m_pCommandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

		BindSamplers(m_defaultSamplers, DEFAULT_SAMPLERS);

		UpdateCameraBuffer();

		m_cameraAndLightBuffers.GetResorce(0).GetResource()->UploadData(&m_camerBufferData, sizeof(CameraBuffer));
		m_cameraAndLightBuffers.GetResorce(1).GetResource()->UploadData(&m_lightBufferData, sizeof(LightBuffer));

		BindBuffers(m_cameraAndLightBuffers, FRAME_CBV);

		return true;
	}

	void WaveManager::EndFrame()
	{
		// Transition the back buffer to present state
		ChangeBackBufferState(STATE_PRESENT);

		HRESULT hr = m_pCommandList->Close();
		WAVEE_ASSERT_MESSAGE(SUCCEEDED(hr), "Failed to close command list!");

		ID3D12CommandList* ppCommandLists[] = { m_pCommandList.Get() };
		m_pCommandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

		// Present the frame.
		hr = m_pSwapChain->Present(1, 0);

		WAVEE_ASSERT_MESSAGE(SUCCEEDED(hr), "Failed to present swap chain!");

		// Schedule a Signal command in the queue.
		const UINT64 currentFenceValue = m_fenceValues[m_frameIndex];
		hr = m_pCommandQueue->Signal(m_pFence.Get(), currentFenceValue);

		// Update the frame index.
		m_frameIndex = m_pSwapChain->GetCurrentBackBufferIndex();

		// If the next frame is not ready to be rendered yet, wait until it is ready.
		if (m_pFence->GetCompletedValue() < m_fenceValues[m_frameIndex])
		{
			hr = m_pFence->SetEventOnCompletion(m_fenceValues[m_frameIndex], m_fenceEvent);
			WAVEE_ASSERT_MESSAGE(SUCCEEDED(hr), "Failed to set on completion event!");
			WaitForSingleObjectEx(m_fenceEvent, INFINITE, FALSE);
		}

		// Set the fence value for the next frame.
		m_fenceValues[m_frameIndex] = currentFenceValue + 1;

		m_uploadManager.EndFrame();
		WInput::Instance()->EndFrameUpdate();

		// Reset current pipeline, material, and mesh
		m_currentPipeline = {};
		m_currentMaterial = {};
		m_currentMesh = {};

		// Reset input states
		UpdateInputStates();
	}

	ResourceID<WSampler> WaveManager::GetDefaultSampler(SamplerType type)
	{
		switch (type)
		{
		case WaveE::WaveManager::WRAP_POINT:
			return m_defaultSamplers.GetResorce(0);
			break;
		case WaveE::WaveManager::WRAP_LINEAR:
			return  m_defaultSamplers.GetResorce(1);
			break;
		case WaveE::WaveManager::CLAMP_POINT:
			return  m_defaultSamplers.GetResorce(2);
			break;
		case WaveE::WaveManager::CLAMP_LINEAR:
			break;
			return  m_defaultSamplers.GetResorce(3);
		default:
			break;
		}
		return  m_defaultSamplers.GetResorce(1);
	}

	void WaveManager::SetLight(Light light, UINT index)
	{
		WAVEE_ASSERT_MESSAGE(index < m_maxLights, "Light index out of range!");
		m_lightBufferData.lights[index] = light;
	}

	void WaveManager::SetAmbientLight(wma::vec4 colour)
	{
		m_lightBufferData.ambientLight = colour;
	}

	void WaveManager::CreateWorldMatrix(wma::mat4& worldMatrix, const WorldMatrixDescriptor& rDescriptor)
	{
		// Initialize the world matrix as an identity matrix
		worldMatrix = wma::mat4{};

		// Apply translation
		worldMatrix = wma::translate(worldMatrix, rDescriptor.worldPos);

		// Apply rotation
		worldMatrix = wma::rotate(worldMatrix, wma::radians(rDescriptor.xRotation), wma::vec3(1.0f, 0.0f, 0.0f));
		worldMatrix = wma::rotate(worldMatrix, wma::radians(rDescriptor.yRotation), wma::vec3(0.0f, 1.0f, 0.0f));
		worldMatrix = wma::rotate(worldMatrix, wma::radians(rDescriptor.zRotation), wma::vec3(0.0f, 0.0f, 1.0f));

		// Apply scaling
		worldMatrix = wma::scale(worldMatrix, wma::vec3(rDescriptor.scale * rDescriptor.xScale, rDescriptor.scale * rDescriptor.yScale, rDescriptor.scale * rDescriptor.zScale));

		worldMatrix = worldMatrix;
	}

	void WaveManager::CopyTexture(ResourceID<WTexture> destination, ResourceID<WTexture> source)
	{
		D3D12_TEXTURE_COPY_LOCATION copyLocationDestination;
		D3D12_TEXTURE_COPY_LOCATION copyLocationSource;

		copyLocationDestination.pResource = destination.GetResource()->GetTexture();
		copyLocationDestination.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
		copyLocationDestination.SubresourceIndex = 0;

		copyLocationSource.pResource = source.GetResource()->GetTexture();
		copyLocationSource.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
		copyLocationSource.SubresourceIndex = 0;

		bool stateChangeSource = false;
		if (source.GetResource()->GetCurrentState() != D3D12_RESOURCE_STATE_COPY_SOURCE)
		{
			stateChangeSource = true;
			D3D12_RESOURCE_BARRIER barrierBeforeCopy = CreateTransitionBarrier(
				source.GetResource()->GetTexture(), source.GetResource()->GetCurrentState(), D3D12_RESOURCE_STATE_COPY_SOURCE);
			m_pCommandList->ResourceBarrier(1, &barrierBeforeCopy);
		}

		bool stateChangeDestination = false;
		if (destination.GetResource()->GetCurrentState() != D3D12_RESOURCE_STATE_COPY_DEST)
		{
			stateChangeDestination = true;
			D3D12_RESOURCE_BARRIER barrierBeforeCopy = CreateTransitionBarrier(
				destination.GetResource()->GetTexture(), destination.GetResource()->GetCurrentState(), D3D12_RESOURCE_STATE_COPY_DEST);
			m_pCommandList->ResourceBarrier(1, &barrierBeforeCopy);
		}

		CopyTexture(&copyLocationDestination, &copyLocationSource);

		if (stateChangeSource)
		{
			D3D12_RESOURCE_BARRIER barrierAfterCopy = CreateTransitionBarrier(
				source.GetResource()->GetTexture(), D3D12_RESOURCE_STATE_COPY_SOURCE, source.GetResource()->GetCurrentState());
			m_pCommandList->ResourceBarrier(1, &barrierAfterCopy);
		}

		if (stateChangeDestination)
		{
			D3D12_RESOURCE_BARRIER barrierAfterCopy = CreateTransitionBarrier(
				destination.GetResource()->GetTexture(), D3D12_RESOURCE_STATE_COPY_DEST, destination.GetResource()->GetCurrentState());
			m_pCommandList->ResourceBarrier(1, &barrierAfterCopy);
		}
	}

	void WaveManager::CopyTexture(const D3D12_TEXTURE_COPY_LOCATION* pDst, const D3D12_TEXTURE_COPY_LOCATION* pSrc)
	{
		m_pCommandList->CopyTextureRegion(pDst, 0, 0, 0, pSrc, nullptr);
	}

	void WaveManager::SetPipelineState(ResourceID<WPipeline> id)
	{
		WAVEE_ASSERT_MESSAGE(id.IsValid(), "Invalid pipeline!");

		if (id.id != m_currentPipeline.id)
		{
			m_pCommandList->SetPipelineState(id.GetResource()->GetPipelineState());
			m_currentPipeline = id;
		}
	}

	void WaveManager::BindBuffer(ResourceID<WBuffer> id, SlotIndex slot)
	{
		WAVEE_ASSERT_MESSAGE(IsCBV_SRV_UAVSlot(slot), "Invalid slot index for buffer!");
		BindResource(id.GetResource()->GetAllocation(), slot);
	}

	void WaveManager::BindTexture(ResourceID<WTexture> id, SlotIndex slot)
	{
		WAVEE_ASSERT_MESSAGE(IsCBV_SRV_UAVSlot(slot), "Invalid slot index for texture!");
		BindResource(id.GetResource()->GetAllocationSRV(), slot);
	}

	void WaveManager::BindSampler(ResourceID<WSampler> id, SlotIndex slot)
	{
		WAVEE_ASSERT_MESSAGE(IsSamplerSlot(slot), "Invalid slot index for sampler!");
		BindResource(id.GetResource()->GetAllocation(), slot);
	}

	void WaveManager::BindBuffers(ResourceBlock<WBuffer> rb, SlotIndex slot)
	{
		WAVEE_ASSERT_MESSAGE(IsCBV_SRV_UAVSlot(slot), "Invalid slot index for buffer!");
		BindResource(rb.allocation, slot);
	}

	void WaveManager::BindTextures(ResourceBlock<WTexture> rb, SlotIndex slot)
	{
		WAVEE_ASSERT_MESSAGE(IsCBV_SRV_UAVSlot(slot), "Invalid slot index for texture!");
		BindResource(rb.allocation, slot);
	}

	void WaveManager::BindSamplers(ResourceBlock<WSampler> rb, SlotIndex slot)
	{
		WAVEE_ASSERT_MESSAGE(IsSamplerSlot(slot), "Invalid slot index for sampler!");
		BindResource(rb.allocation, slot);
	}

	void WaveManager::BindResource(WDescriptorHeapManager::Allocation allocation, SlotIndex slot)
	{
		if (IsCBV_SRV_UAVSlot(slot))
		{
			m_pCommandList->SetGraphicsRootDescriptorTable(slot, m_cbvSrvUavHeap.GetGPUHandle(allocation));
			return;
		}
		if (IsSamplerSlot(slot))
		{
			m_pCommandList->SetGraphicsRootDescriptorTable(slot, m_samplerHeap.GetGPUHandle(allocation));
			return;
		}

		WAVEE_ASSERT_MESSAGE(false, "Invalid slot index!");
	}

	void WaveManager::SetRenderTarget(ResourceID<WTexture> RTVId, ResourceID<WTexture> DSVId)
	{
		D3D12_VIEWPORT viewport;
		viewport.TopLeftX = 0;
		viewport.TopLeftY = 0;
		viewport.Width = static_cast<float>(RTVId.GetResource()->GetWidth());
		viewport.Height = static_cast<float>(RTVId.GetResource()->GetHeight());
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;

		m_pCommandList->RSSetViewports(1, &viewport);

		D3D12_RECT scissorRect;
		scissorRect.left = 0;
		scissorRect.top = 0;
		scissorRect.right = static_cast<LONG>(RTVId.GetResource()->GetWidth());;
		scissorRect.bottom = static_cast<LONG>(RTVId.GetResource()->GetHeight());

		m_pCommandList->RSSetScissorRects(1, &scissorRect);

		bool shouldSetRTV = RTVId.id != UINT_MAX;
		bool shouldSetDSV = DSVId.id != UINT_MAX;

		D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle;
		D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle;

		if (shouldSetRTV)
		{
			rtvHandle = m_rtvHeap.GetCPUHandle(RTVId.GetResource()->GetAllocationRTV_DSV());
		}
		if (shouldSetDSV)
		{
			dsvHandle = m_dsvHeap.GetCPUHandle(DSVId.GetResource()->GetAllocationRTV_DSV());
		}

		m_pCommandList->OMSetRenderTargets(1, shouldSetRTV ? &rtvHandle : nullptr, FALSE, shouldSetDSV ? &dsvHandle : nullptr);
	}

	void WaveManager::SetRenderTargetToSwapChain(ResourceID<WTexture> DSVId)
	{
		D3D12_VIEWPORT viewport;
		viewport.TopLeftX = 0;
		viewport.TopLeftY = 0;
		viewport.Width = static_cast<float>(GetWidth());
		viewport.Height = static_cast<float>(GetHeight());
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;

		m_pCommandList->RSSetViewports(1, &viewport);

		D3D12_RECT scissorRect;
		scissorRect.left = 0;
		scissorRect.top = 0;
		scissorRect.right = static_cast<LONG>(GetWidth());;
		scissorRect.bottom = static_cast<LONG>(GetHeight());

		m_pCommandList->RSSetScissorRects(1, &scissorRect);

		D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle;
		D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle;

		rtvHandle = m_rtvHeap.GetCPUHandle(m_backBufferAllocations[m_frameIndex]);

		bool shouldSetDSV = DSVId.id != UINT_MAX;
		if (shouldSetDSV)
		{
			dsvHandle = m_dsvHeap.GetCPUHandle(DSVId.GetResource()->GetAllocationRTV_DSV());
		}

		// Transition the back buffer to a render target
		ChangeBackBufferState(STATE_TARGET);

		m_pCommandList->OMSetRenderTargets(1, &rtvHandle, FALSE, shouldSetDSV ? &dsvHandle : nullptr);
	}

	void WaveManager::ClearRenderTarget(ResourceID<WTexture> id, wma::vec4 colour)
	{
		WAVEE_ASSERT_MESSAGE(!id.GetResource()->IsDepthType(), "Can't clear render target view on depth texture!");

		D3D12_CPU_DESCRIPTOR_HANDLE handle = m_rtvHeap.GetCPUHandle(id.GetResource()->GetAllocationRTV_DSV());
		m_pCommandList->ClearRenderTargetView(handle, &colour[0], 0, nullptr);
	}

	void WaveManager::ClearDepthStencilTarget(ResourceID<WTexture> id, float depth, UINT stencil)
	{
		WAVEE_ASSERT_MESSAGE(id.GetResource()->IsDepthType(), "Can't clear depth stencil target view on non depth texture!");

		D3D12_CPU_DESCRIPTOR_HANDLE handle = m_dsvHeap.GetCPUHandle(id.GetResource()->GetAllocationRTV_DSV());
		m_pCommandList->ClearDepthStencilView(handle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, depth, stencil, 0, nullptr);
	}

	void WaveManager::ClearBackBuffer(wma::vec4 colour)
	{
		// Transition the back buffer to target state
		ChangeBackBufferState(STATE_TARGET);

		D3D12_CPU_DESCRIPTOR_HANDLE handle = m_rtvHeap.GetCPUHandle(m_backBufferAllocations[m_frameIndex]);
		m_pCommandList->ClearRenderTargetView(handle, &colour[0], 0, nullptr);
	}

	void WaveManager::DrawMeshWithCurrentParamaters(ResourceID<WMesh> id, UINT count /*= 1*/)
	{
		if (id.id != m_currentMesh.id)
		{
			id.GetResource()->Bind();
			m_currentMesh = id;
		}
		m_pCommandList->DrawInstanced(id.GetResource()->GetVertexCount(), count, 0, 0);
	}

	void WaveManager::DrawIndexedMeshWithCurrentParamaters(ResourceID<WMesh> id, UINT count /*= 1*/)
	{
		if (id.id != m_currentMesh.id)
		{
			id.GetResource()->Bind();
			m_currentMesh = id;
		}
		m_pCommandList->DrawIndexedInstanced(id.GetResource()->GetIndexCount(), count, 0, 0, 0);
	}

	void WaveManager::DrawMesh(ResourceID<WMesh> mesh, ResourceID<WMaterial> material, UINT count)
	{
		if (mesh.id != m_currentMesh.id)
		{
			mesh.GetResource()->Bind();
			m_currentMesh = mesh;
		}
		if (material.id != m_currentMaterial.id)
		{
			material.GetResource()->BindMaterial();
			m_currentMaterial = material;
		}
		m_pCommandList->DrawInstanced(mesh.GetResource()->GetVertexCount(), count, 0, 0);
	}

	void WaveManager::DrawIndexedMesh(ResourceID<WMesh> mesh, ResourceID<WMaterial> material, UINT count)
	{
		if (mesh.id != m_currentMesh.id)
		{
			mesh.GetResource()->Bind();
			m_currentMesh = mesh;
		}
		if (material.id != m_currentMaterial.id)
		{
			material.GetResource()->BindMaterial();
			m_currentMaterial = material;
		}
		m_pCommandList->DrawIndexedInstanced(mesh.GetResource()->GetIndexCount(), count, 0, 0, 0);
	}

	void WaveManager::SetRootSigniture(WRootSigniture* pRootSigniture)
	{
		m_pCommandList->SetGraphicsRootSignature(pRootSigniture->GetRootSignature());
	}

	void WaveManager::GetHardwareAdapter(IDXGIFactory1* pFactory, IDXGIAdapter1** ppAdapter, bool requestHighPerformanceAdapter)
	{
		*ppAdapter = nullptr;

		ComPtr<IDXGIAdapter1> adapter;

		ComPtr<IDXGIFactory6> factory6;
		if (SUCCEEDED(pFactory->QueryInterface(IID_PPV_ARGS(&factory6))))
		{
			for (
				UINT adapterIndex = 0;
				SUCCEEDED(factory6->EnumAdapterByGpuPreference(
					adapterIndex,
					requestHighPerformanceAdapter == true ? DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE : DXGI_GPU_PREFERENCE_UNSPECIFIED,
					IID_PPV_ARGS(&adapter)));
					++adapterIndex)
			{
				DXGI_ADAPTER_DESC1 desc;
				adapter->GetDesc1(&desc);

				if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
				{
					// Don't select the Basic Render Driver adapter.
					continue;
				}

				// Check to see whether the adapter supports Direct3D 12, but don't create the
				// actual device yet.
				if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
				{
					break;
				}
			}
		}
	}

	void WaveManager::CreateDefaultRootSigniture()
	{
		constexpr UINT numDescriptorTables = 7;
		WRootSigniture::DescriptorTable descriptorTables[numDescriptorTables];
		// Default Samplers
		descriptorTables[0].numSamplers = m_defaultSamplerCount;
		
		// Per frame constants
		descriptorTables[1].numCBVs = m_frameCVBCount;

		// Per draw constants
		descriptorTables[2].numCBVs = m_drawCBVCount;

		// Global
		descriptorTables[3].numCBVs = m_globalCBVCount;
		descriptorTables[3].numSRVs = m_globalSRVCount;
		descriptorTables[4].numSamplers = m_globalSamplerCount;

		// Material
		descriptorTables[5].numCBVs = m_materialCBVCount;
		descriptorTables[5].numSRVs = m_materialSRVCount;
		descriptorTables[6].numSamplers = m_materialSamplerCount;

		WRootSigniture::RootSignatureDescriptor rootSignitureDescriptor;
		rootSignitureDescriptor.descriptorTables = descriptorTables;
		rootSignitureDescriptor.numDescriptorTables = numDescriptorTables;

		m_defaultRootSigniture.CreateRootSigniture(rootSignitureDescriptor);
	}

	void WaveManager::CreateSlotHLSLIFile()
	{
		std::ofstream file("SlotMacros.hlsli");
		WAVEE_ASSERT_MESSAGE(file.is_open(), "Could not create hlsli file");

		file << "#pragma once\n";

		UINT CBVCount = 0;
		UINT SRVCount = 0;
		UINT samplerCount = 0;

		// Writing default slots
		for (UINT i = 0; i < m_defaultSamplerCount; ++i)
		{
			file << "#define DEFAULT_SAMPLER_" << i << " s" << i + samplerCount << "\n";
		}
		samplerCount += m_defaultSamplerCount;

		// Writing per frame slots
		for (UINT i = 0; i < m_frameCVBCount; ++i)
		{
			file << "#define FRAME_CBV_" << i << " b" << i + CBVCount << "\n";
		}
		CBVCount += m_frameCVBCount;

		// Writing per draw slots
		for (UINT i = 0; i < m_drawCBVCount; ++i)
		{
			file << "#define DRAW_CBV_" << i << " b" << i + CBVCount << "\n";
		}
		CBVCount += m_drawCBVCount;

		// Writing global slots
		for (UINT i = 0; i < m_globalCBVCount; ++i)
		{
			file << "#define GLOBAL_CBV_" << i << " b" << i + CBVCount << "\n";
		}
		CBVCount += m_globalCBVCount;

		for (UINT i = 0; i < m_globalSRVCount; ++i)
		{
			file << "#define GLOBAL_SRV_" << i << " t" << i + SRVCount << "\n";
		}
		SRVCount += m_globalSRVCount;

		for (UINT i = 0; i < m_globalSamplerCount; ++i)
		{
			file << "#define GLOBAL_SAMPLER_" << i << " s" << i + samplerCount << "\n";
		}
		samplerCount += m_globalSamplerCount;

		// Writing material slots
		for (UINT i = 0; i < m_materialCBVCount; ++i)
		{
			file << "#define MATERIAL_CBV_" << i << " b" << i + CBVCount << "\n";
		}
		CBVCount += m_materialCBVCount;

		for (UINT i = 0; i < m_materialSRVCount; ++i)
		{
			file << "#define MATERIAL_SRV_" << i << " t" << i + SRVCount << "\n";
		}
		SRVCount += m_materialSRVCount;

		for (UINT i = 0; i < m_materialSamplerCount; ++i)
		{
			file << "#define MATERIAL_SAMPLER_" << i << " s" << i + samplerCount << "\n";
		}
		samplerCount += m_materialSamplerCount;

		file.close();
	}

	std::string WaveManager::GetResourceDirectory()
	{
		if (m_resourceDirectory.size() == 0)
		{
			 m_resourceDirectory = std::filesystem::current_path().string() + "\\..\\Resources";
		}
		return m_resourceDirectory;
	}

	std::string WaveManager::GetShaderDirectory()
	{
		std::string pathString{ GetResourceDirectory() };
		pathString += "\\Shaders";
		
#ifdef _DEBUG
		pathString += "\\Debug";
#else
		pathString += "\\Release";
#endif
		return pathString;
	}

	std::string WaveManager::GetTextureDirectory()
	{
		std::string pathString{ GetResourceDirectory() };
		pathString += "\\Textures";

		return m_resourceDirectory;
	}

	std::string WaveManager::GetMeshDirectory()
	{
		std::string pathString{ GetResourceDirectory() };
		pathString += "\\Meshes";

		return m_resourceDirectory;
	}

	bool WaveManager::IsSamplerSlot(SlotIndex index)
	{
		return index == GLOBAL_SAMPLERS || index == DEFAULT_SAMPLERS || index == MATERIAL_SAMPLERS;
	}

	bool WaveManager::IsCBV_SRV_UAVSlot(SlotIndex index)
	{
		return index == GLOBAL_CBV_SRV || index == MATERIAL_CBV_SRV|| index == FRAME_CBV || index == DRAW_CBV;
	}

	bool WaveManager::ChangeBackBufferState(BackBufferState state)
	{
		if (m_backBufferStates[m_frameIndex] != state)
		{
			D3D12_RESOURCE_BARRIER barrier = CreateTransitionBarrier(
				m_pBackBuffers[m_frameIndex].Get(),
				state == STATE_PRESENT ? D3D12_RESOURCE_STATE_RENDER_TARGET : D3D12_RESOURCE_STATE_PRESENT,
				state == STATE_TARGET ? D3D12_RESOURCE_STATE_RENDER_TARGET : D3D12_RESOURCE_STATE_PRESENT);

			m_pCommandList->ResourceBarrier(1, &barrier);
			m_backBufferStates[m_frameIndex] = state;

			return true;
		}
		return false;
	}

	__int64 WaveManager::GetTime()
	{
		__int64 time;

		QueryPerformanceCounter((LARGE_INTEGER*)& time);

		return time;
	}

	void WaveManager::InitTime()
	{
		__int64 countsPerSec;
		QueryPerformanceFrequency((LARGE_INTEGER*)&countsPerSec);
		m_secondsPerCount = 1.0 / static_cast<double>(countsPerSec);

		m_startTime = GetTime();
	}

	void WaveManager::UpdateTime()
	{
		// Update delta time
		__int64 newTime = GetTime();

		m_deltaTime = (newTime - m_currentTime) * m_secondsPerCount;


		if (m_deltaTime < 0.0)
		{
			m_deltaTime = 0.0;
		}

		// Sleep if delta time is less that frametime
		double timeToWait = m_targetFrameTime - m_deltaTime;

		if (timeToWait > 0.0)
		{
			// Convert time to wait from seconds to milliseconds
			DWORD sleepTime = static_cast<DWORD>(timeToWait * 1000.0);

			// Sleep for the remaining time
			Sleep(sleepTime);

			newTime = GetTime();

			m_deltaTime = (newTime - m_currentTime) * m_secondsPerCount;

			if (m_deltaTime < 0.0)
			{
				m_deltaTime = 0.0;
			}
		}

		m_currentTime = newTime;

		m_gameTime = (m_currentTime - m_startTime) * m_secondsPerCount;

	}

	void WaveManager::UpdateCameraBuffer()
	{
		m_camerBufferData.viewMatrix = m_gameCamera.GetViewMatrix();
		m_camerBufferData.projectionMatrix = m_gameCamera.GetProjectionMatrix();
		m_camerBufferData.inverseProjectionMatrix = wma::inverse(m_camerBufferData.projectionMatrix);
		m_camerBufferData.viewPos = wma::vec4{ m_gameCamera.GetPosition(), 0 };
		m_camerBufferData.time[0] = static_cast<float>(GetGameTime());
	}

	void WaveManager::UpdateInputStates()
	{
		m_previousMouseState = m_currentMouseState;
		m_previousKeyState = m_currentKeyState;

		m_currentMouseState.delta.x = 0;
		m_currentMouseState.delta.y = 0;
	}

	void WaveManager::UpdateGameCamera()
	{
		if (WInput::Instance()->IsKeyDown(m_gameCameraControls.forward))
		{
			m_gameCamera.MoveForward(m_gameCameraControls.moveSpeed * GetDeltaTime());
		}
		if (WInput::Instance()->IsKeyDown(m_gameCameraControls.backwards))
		{
			m_gameCamera.MoveForward(-m_gameCameraControls.moveSpeed * GetDeltaTime());
		}
		if (WInput::Instance()->IsKeyDown(m_gameCameraControls.right))
		{
			m_gameCamera.MoveRight(m_gameCameraControls.moveSpeed * GetDeltaTime());
		}
		if (WInput::Instance()->IsKeyDown(m_gameCameraControls.left))
		{
			m_gameCamera.MoveRight(-m_gameCameraControls.moveSpeed * GetDeltaTime());
		}
		if (WInput::Instance()->IsKeyDown(m_gameCameraControls.up))
		{
			m_gameCamera.MoveUp(m_gameCameraControls.moveSpeed * GetDeltaTime());
		}
		if (WInput::Instance()->IsKeyDown(m_gameCameraControls.down))
		{
			m_gameCamera.MoveUp(-m_gameCameraControls.moveSpeed * GetDeltaTime());
		}
		m_gameCamera.Rotate(WInput::Instance()->GetMouseDelta().x * m_gameCameraControls.mouseSensitivity, WInput::Instance()->GetMouseDelta().y * m_gameCameraControls.mouseSensitivity);
	}
}
