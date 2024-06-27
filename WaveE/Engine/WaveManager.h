#pragma once
#include <functional>
#include <bitset>

#include "WInput.h"
#include "WDescriptorHeapManager.h"
#include "WUploadManager.h"
#include "WRootSigniture.h"
#include "WSampler.h"
#include "WResourceManager.h"
#include "WCamera.h"
#include "WMaterial.h"


namespace WaveE
{
	struct CameraControls
	{
		float mouseSensitivity{ 1.f };
		float moveSpeed{ 5 };
		UINT forward{ 'W' };
		UINT backwards{ 'S' };
		UINT left{ 'A' };
		UINT right{ 'D' };
		UINT up{ 'Q' };
		UINT down{ 'E' };
	};

	struct WaveEDescriptor
	{
		UINT width{ 1280 };
		UINT height{ 720 };
		float targetFrameRate{ 60.f };
		const char* title{ "WaveE" };
		CameraControls cameraControls{};
	};

	// Manager class for WaveE
	class WaveManager
	{
		// Singleton setup
		WaveManager(const WaveManager&) = delete; 
		WaveManager& operator=(const WaveManager&) = delete; 
	public: 
		static WaveManager* Instance() { return ms_pInstance; }

		static void Init(const WaveEDescriptor& rDescriptor); 
		static void EndInit();
		static void Uninit(); 

	private: 
		static WaveManager* ms_pInstance;

	public:
		struct MouseState
		{
			POINTS mousePosition;
			wma::vec2 delta;
			bool leftMouseButton;
			bool middleMouseButton;
			bool rightMouseButton;
		};

		bool BeginFrame();
		void EndFrame();

		// Getter functions
		WaveEDevice* GetDevice() const { return m_pDevice.Get(); }
		WaveECommandList* GetCommandList() const { return m_pCommandList.Get(); }
		WaveECommandQueue* GetCommandQueue() const { return m_pCommandQueue.Get(); }
		WaveESwapChain* GetSwapChain() const { return m_pSwapChain.Get(); }
		ID3D12Resource* GetCurrentBackBuffer() const { return m_pBackBuffers[m_frameIndex].Get(); }

		WDescriptorHeapManager* GetCBV_SRV_UAVHeap() { return &m_cbvSrvUavHeap; }
		WDescriptorHeapManager* GetRTVHeap() { return &m_rtvHeap; }
		WDescriptorHeapManager* GetDSVHeap() { return &m_dsvHeap; }
		WDescriptorHeapManager* GetSamplerHeap() { return &m_samplerHeap; }

		WUploadManager* GetUploadManager() { return &m_uploadManager; }

		ResourceID<WPipeline> GetDefaultPipelineState() const { return m_defaultPipeline3D; }
		WRootSigniture* GetDefaultRootSigniture() { return &m_defaultRootSigniture; }
		D3D12_INPUT_LAYOUT_DESC GetDefaultInputLayout() const { return m_defaultInputLayout; }
		ResourceID<WTexture> GetDefaultDepthTexture() const { return m_defaultDepthTexture; }

		UINT GetWidth() const { return m_width; }
		UINT GetHeight() const { return m_height; }

		WCamera& GetGameCamera() { return m_gameCamera; }
		
		double GetDeltaTime() const { return m_deltaTime; }
		double GetGameTime() const { return m_gameTime; }

		const MouseState& GetMouseState(bool getPrevious = false) const{ return getPrevious ? m_previousMouseState : m_currentMouseState; }
		const std::bitset<256>& GetKeyState(bool getPrevious = false) const{ return getPrevious ? m_previousKeyState : m_currentKeyState; }

		enum SamplerType
		{
			WRAP_POINT,
			WRAP_LINEAR,
			CLAMP_POINT,
			CLAMP_LINEAR
		};
		ResourceID<WSampler> GetDefaultSampler(SamplerType type = WRAP_LINEAR);

		// Game function

		static constexpr UINT m_maxLights{ 4 };

		struct Light
		{
			wma::vec4 position{ 0,0,0,0 };
			wma::vec4 colour{ 0,0,0,0 };
		};

		void SetLight(Light light, UINT index);
		void SetAmbientLight(wma::vec4 colour);

		struct WorldMatrixDescriptor
		{
			wma::vec3 worldPos{ 0 };
			float scale{ 1 };
			float xScale{ 1 };
			float yScale{ 1 };
			float zScale{ 1 };
			float xRotation{ 0 };
			float yRotation{ 0 };
			float zRotation{ 0 };
		};
		void CreateWorldMatrix(wma::mat4& worldMatrix, const WorldMatrixDescriptor& rDescriptor);

		// Drawing functions
		enum SlotIndex : UINT
		{
			DEFAULT_SAMPLERS = 0,
			FRAME_CBV, // Camera and light buffers
			DRAW_CBV, // World matrix
			GLOBAL_CBV_SRV,
			GLOBAL_SAMPLERS,
			MATERIAL_CBV_SRV,
			MATERIAL_SAMPLERS
		};
		void BindBuffer(ResourceID<WBuffer> id, SlotIndex slot);
		void BindTexture(ResourceID<WTexture> id, SlotIndex slot);
		void BindSampler(ResourceID<WSampler> id, SlotIndex slot);
		void BindBuffers(ResourceBlock<WBuffer> rb, SlotIndex slot);
		void BindTextures(ResourceBlock<WTexture> rb, SlotIndex slot);
		void BindSamplers(ResourceBlock<WSampler> rb, SlotIndex slot);
		void BindResource(WDescriptorHeapManager::Allocation allocation, SlotIndex slot);

		// Sets the render target, viewport, and scissor rect for drawing to the whole screen
		void SetRenderTarget(ResourceID<WTexture> RTVId, ResourceID<WTexture> DSVId = {});
		void SetRenderTargetToSwapChain(ResourceID<WTexture> DSVId = {});
		void ClearRenderTarget(ResourceID<WTexture> id, wma::vec4 colour = { 0,0,0,1 });
		void ClearDepthStencilTarget(ResourceID<WTexture> id, float depth = 1, UINT stencil = 0);
		void ClearBackBuffer(wma::vec4 colour = { 0,0,0,1 });

		void CopyTexture(ResourceID<WTexture> destination, ResourceID<WTexture> source);
		void CopyTexture(const D3D12_TEXTURE_COPY_LOCATION* pDst, const D3D12_TEXTURE_COPY_LOCATION* pSrc);

		void SetPipelineState(ResourceID<WPipeline> id);

		void SetDefaultRootSigniture() { SetRootSigniture(&m_defaultRootSigniture); }
		void SetRootSigniture(WRootSigniture* pRootSigniture);

		void DrawMeshWithCurrentParamaters(ResourceID<WMesh> id, UINT count = 1);
		void DrawIndexedMeshWithCurrentParamaters(ResourceID<WMesh> id, UINT count = 1);

		void DrawMesh(ResourceID<WMesh> mesh, ResourceID<WMaterial> material, UINT count = 1);
		void DrawIndexedMesh(ResourceID<WMesh> mesh, ResourceID<WMaterial> material, UINT count = 1);

	private:
		enum BackBufferState
		{
			STATE_PRESENT,
			STATE_TARGET
		};

		struct CameraBuffer
		{
			wma::mat4 viewMatrix;
			wma::mat4 projectionMatrix;
			wma::mat4 inverseProjectionMatrix;
			wma::vec4 viewPos;
			wma::vec4 time;
		};

		struct LightBuffer
		{
			wma::vec4 ambientLight;
			Light lights[m_maxLights];
		};

		WaveManager(const WaveEDescriptor& rDescriptor);
		~WaveManager();

		void InitWindow(const WaveEDescriptor& rDescriptor);
		void InitDX12(const WaveEDescriptor& rDescriptor);

		// Updates the window loop, return true if the program should quit
		bool UpdateWindowLoop();
		static LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

		void HideCursor();
		void ShowCursor();
		void ConfineCursor();
		void ReleaseCursor();

		void HandleKeyEvent(WPARAM wParam, bool isDown);
		void HandleMouseMove(UINT message, WPARAM wParam, LPARAM lParam);
		void HandleRawInput(LPARAM lParam);

		void GetHardwareAdapter(
			IDXGIFactory1* pFactory,
			IDXGIAdapter1** ppAdapter,
			bool requestHighPerformanceAdapter = false);

		void CreateDefaultRootSigniture();

		// Create the hlsl include file for named slots
		// If the default slot layout changes the shaders don't need to be changes
		// Just replace the old hlsli file with the new one and recompile
		void CreateSlotHLSLIFile();

		std::string GetResourceDirectory();
		std::string GetShaderDirectory();
		std::string GetTextureDirectory();
		std::string GetMeshDirectory();
		std::string m_resourceDirectory;

		bool IsSamplerSlot(SlotIndex index);
		bool IsCBV_SRV_UAVSlot(SlotIndex index);

		bool ChangeBackBufferState(BackBufferState state);

		__int64 GetTime();
		void InitTime();
		void UpdateTime();

		void UpdateCameraBuffer();

		void UpdateInputStates();
		void UpdateGameCamera();

		// DX12 variables
		static const UINT m_frameCount{ 2 };

		ComPtr<WaveEDevice> m_pDevice;
		ComPtr<WaveECommandList> m_pCommandList;
		ComPtr<WaveECommandQueue> m_pCommandQueue;
		ComPtr<WaveESwapChain> m_pSwapChain;
		ComPtr<ID3D12CommandAllocator> m_pCommandAllocators[m_frameCount];
		ComPtr<ID3D12Resource> m_pBackBuffers[m_frameCount];
		WDescriptorHeapManager::Allocation m_backBufferAllocations[m_frameCount];
		BackBufferState m_backBufferStates[m_frameCount];

		UINT m_frameIndex;
		HANDLE m_fenceEvent;
		ComPtr<ID3D12Fence> m_pFence;
		UINT64 m_fenceValues[m_frameCount];

		// Windows variables
		HWND m_hwnd{ NULL };
		HINSTANCE m_hInstance{ GetModuleHandle(NULL) };

		// Input
		// Keyboard state
		std::bitset<256> m_currentKeyState;
		std::bitset<256> m_previousKeyState;
		// Mouse State
		MouseState m_currentMouseState;
		MouseState m_previousMouseState;

		// WaveE variables
		WDescriptorHeapManager m_cbvSrvUavHeap;
		WDescriptorHeapManager m_rtvHeap;
		WDescriptorHeapManager m_dsvHeap;
		WDescriptorHeapManager m_samplerHeap;

		WUploadManager m_uploadManager;

		WRootSigniture m_defaultRootSigniture;

		ResourceBlock<WSampler> m_defaultSamplers;

		D3D12_INPUT_ELEMENT_DESC m_defaultInputElements[3];
		D3D12_INPUT_LAYOUT_DESC m_defaultInputLayout;
		
		ResourceID<WPipeline> m_defaultPipeline3D;
		ResourceID<WTexture> m_defaultDepthTexture;
		static constexpr char* m_defaultPixelShaderName{ "SimpleLighting_PS" };
		static constexpr char* m_defaultVertexShaderName{ "SimpleLighting_VS" };
		static constexpr DXGI_FORMAT m_defaultDepthType{ DXGI_FORMAT_D32_FLOAT };

		ResourceID<WPipeline> m_currentPipeline{};
		ResourceID<WMaterial> m_currentMaterial{};
		ResourceID<WMesh> m_currentMesh{};

		UINT m_width;
		UINT m_height;
		// #TODO Implement window size change logic
		bool m_hasWindowSizeChanged{ false };

		ResourceBlock<WBuffer> m_cameraAndLightBuffers;

		// Game variables
		double m_gameTime{ 0 };
		double m_deltaTime{ 0 };
		double m_secondsPerCount;
		__int64  m_startTime{ 0 };
		__int64 m_currentTime{ 0 };
		float m_targetFrameTime;

		CameraControls m_gameCameraControls;
		WCamera m_gameCamera;

		CameraBuffer m_camerBufferData;
		LightBuffer m_lightBufferData;

		// Constants
		const UINT m_descriptorHeapCountCBV_SRV_UAV{ 1024 };
		const UINT m_descriptorHeapCountRTV{ 32 };
		const UINT m_descriptorHeapCountDSV{ 32 };
		const UINT m_descriptorHeapCountSampler{ 32 };

		const UINT m_bigUploadBufferCount{ 10 };
		const UINT m_smallUploadBufferCount{ 50 };
		const size_t m_bigUploadBufferSize{ 1024 * 1024 * 32 * 2 };
		const size_t m_smallUploadBufferSize{ 1024 * 32 };

		const UINT m_defaultSamplerCount{ 4 };
		const UINT m_frameCVBCount{ 2 };
		const UINT m_drawCBVCount{ 1 };
		const UINT m_globalCBVCount{ 1 };
		const UINT m_globalSRVCount{ 8 };
		const UINT m_globalSamplerCount{ m_globalSRVCount };
		const UINT m_materialCBVCount{ 1 };
		const UINT m_materialSRVCount{ 4 };
		const UINT m_materialSamplerCount{ m_materialSRVCount };
	};
}

