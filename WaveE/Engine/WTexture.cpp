#include "stdafx.h"
#include "WTexture.h"
#include "WaveManager.h"

namespace WaveE
{
	DXGI_FORMAT GetDXGIFormat(WTextureDescriptor::Format format)
	{
		switch (format)
		{
		case WTextureDescriptor::RGBA:
			return DXGI_FORMAT_R8G8B8A8_UNORM;
		case WTextureDescriptor::SRGBA:
			return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
		case WTextureDescriptor::RGBAF16:
			return DXGI_FORMAT_R16G16B16A16_FLOAT;
		case WTextureDescriptor::DepthFloat:
			return DXGI_FORMAT_D32_FLOAT;
		case WTextureDescriptor::DepthTypeless:
			return DXGI_FORMAT_R32_TYPELESS;
		default:
			return DXGI_FORMAT_UNKNOWN;
		}
	}

	DXGI_FORMAT GetDXGIFormatForDepth(WTextureDescriptor::Format format, bool isForSRV = true)
	{
		switch (format)
		{
		case WTextureDescriptor::DepthTypeless: [[fallthrough]];
		case WTextureDescriptor::DepthFloat:
			return isForSRV ? DXGI_FORMAT_R32_FLOAT : DXGI_FORMAT_D32_FLOAT;
		default:
			return DXGI_FORMAT_UNKNOWN;
		}
	}

	size_t GetBytesPerPixel(DXGI_FORMAT format)
	{
		switch (format)
		{
		case DXGI_FORMAT_R8G8B8A8_UNORM: [[fallthrough]];
		case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB: [[fallthrough]];
		case DXGI_FORMAT_D32_FLOAT: [[fallthrough]];
		case DXGI_FORMAT_R32_TYPELESS: return 4;
		case DXGI_FORMAT_R16G16B16A16_FLOAT: return 8;
		default:
			WAVEE_ASSERT_MESSAGE(false, "Format not supported!");
		}

		return -1;
	}

	D3D12_RESOURCE_FLAGS GetResourceFlags(WTextureDescriptor::Usage usage, WTextureDescriptor::Format format)
	{
		D3D12_RESOURCE_FLAGS flags = {};

		if (usage & WTextureDescriptor::Usage::RenderTarget)
		{
			if (format == WTextureDescriptor::Format::DepthFloat || format == WTextureDescriptor::Format::DepthTypeless)
			{
				flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
			}
			else
			{
				flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
			}
		}
		if (!(usage & WTextureDescriptor::Usage::ShaderResource) && (format == WTextureDescriptor::DepthFloat || format == WTextureDescriptor::DepthTypeless))
		{
			flags |= D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE;
		}

		return flags;
	}

	D3D12_RESOURCE_STATES GetResourceState(WTextureDescriptor::Format format, bool isShaderResource)
	{
		if (isShaderResource)
		{;

			switch (format)
			{
			case WaveE::WTextureDescriptor::RGBA: [[fallthrough]];
			case WaveE::WTextureDescriptor::SRGBA: [[fallthrough]];
			case WaveE::WTextureDescriptor::RGBAF16:
				return D3D12_RESOURCE_STATE_GENERIC_READ;
			case WaveE::WTextureDescriptor::DepthFloat: [[fallthrough]];
			case WaveE::WTextureDescriptor::DepthTypeless:
				return D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
			}
		}
		else
		{
			switch (format)
			{
			case WaveE::WTextureDescriptor::RGBA: [[fallthrough]];
			case WaveE::WTextureDescriptor::SRGBA: [[fallthrough]];
			case WaveE::WTextureDescriptor::RGBAF16:
				return D3D12_RESOURCE_STATE_RENDER_TARGET;
			case WaveE::WTextureDescriptor::DepthFloat: [[fallthrough]];
			case WaveE::WTextureDescriptor::DepthTypeless:
				return D3D12_RESOURCE_STATE_DEPTH_WRITE;
			}
		}

		WAVEE_ASSERT_MESSAGE(false, "Did not find resourceState");
		return D3D12_RESOURCE_STATE_GENERIC_READ;
	}

	WTexture::WTexture(const WTextureDescriptor& rDescriptor, WDescriptorHeapManager::Allocation allocationSRV, UINT offset)
		: m_allocationSRV{ allocationSRV }
		, m_offsetSRV{offset}
		, m_doesOwnAllocationSRV{ WDescriptorHeapManager::IsInvalidAllocation(allocationSRV) }
		, m_allocationRTV_DSV{ WDescriptorHeapManager::InvalidAllocation() }
	{
		m_width = rDescriptor.width;
		m_height = rDescriptor.height;
		m_currentState = rDescriptor.startAsShaderResource ? State::Input : State::Output;

		m_isDepthType = rDescriptor.format == WTextureDescriptor::Format::DepthFloat || rDescriptor.format == WTextureDescriptor::Format::DepthTypeless;

		D3D12_HEAP_PROPERTIES heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
		DXGI_FORMAT dxgiFormat = GetDXGIFormat(rDescriptor.format);
		DXGI_FORMAT dxgiFormatNonTypelessForDepthSRV = GetDXGIFormatForDepth(rDescriptor.format, true);
		DXGI_FORMAT dxgiFormatNonTypelessForDepthDSV = GetDXGIFormatForDepth(rDescriptor.format, false);
		D3D12_RESOURCE_FLAGS resourceFlags = GetResourceFlags(rDescriptor.usage, rDescriptor.format);

		m_bytesPerPixel = GetBytesPerPixel(dxgiFormat);
		m_sizeBytes = m_width * m_height * m_bytesPerPixel;

		m_renderTargetState = GetResourceState(rDescriptor.format, false);
		m_shaderResourceState = GetResourceState(rDescriptor.format, true);

		D3D12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Tex2D(dxgiFormat, rDescriptor.width, rDescriptor.height);
		resourceDesc.Flags = resourceFlags;
		resourceDesc.MipLevels = 1;

		D3D12_CLEAR_VALUE clearValue = {};
		if (m_isDepthType)
		{
			clearValue.Format = dxgiFormatNonTypelessForDepthDSV;
		}
		else
		{
			clearValue.Format = dxgiFormat;
		}

		if (m_isDepthType)
		{
			clearValue.DepthStencil.Depth = 1.0f;
			clearValue.DepthStencil.Stencil = 0;
		}
		else
		{
			clearValue.Color[0] = 0.0f;
			clearValue.Color[1] = 0.0f;
			clearValue.Color[2] = 0.0f;
			clearValue.Color[3] = 0.0f;
		}

		D3D12_RESOURCE_STATES initialState;
		D3D12_RESOURCE_STATES finalState = m_currentState == State::Input ? m_shaderResourceState : m_renderTargetState;

		if (rDescriptor.pInitalData)
		{
			initialState = D3D12_RESOURCE_STATE_COPY_DEST;
		}
		else
		{
			initialState = finalState;
		}

		WaveEDevice* pDevice = WaveManager::Instance()->GetDevice();

		bool useClearValue = rDescriptor.usage & WTextureDescriptor::RenderTarget;

		HRESULT hr = pDevice->CreateCommittedResource(
			&heapProperties,
			D3D12_HEAP_FLAG_NONE,
			&resourceDesc,
			initialState,
			useClearValue ? &clearValue : nullptr,
			IID_PPV_ARGS(&m_pTexture)
		);

		WAVEE_ASSERT_MESSAGE(SUCCEEDED(hr), "Failed to create committed resource for texture!");

		if (rDescriptor.usage & WTextureDescriptor::Usage::RenderTarget)
		{
			if (m_isDepthType)
			{
				D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
				dsvDesc.Format = dxgiFormatNonTypelessForDepthDSV;
				dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
				dsvDesc.Texture2D.MipSlice = 0;

				WDescriptorHeapManager* pDSVDescriptorHeapManager = WaveManager::Instance()->GetDSVHeap();
				m_allocationRTV_DSV = pDSVDescriptorHeapManager->Allocate();
				D3D12_CPU_DESCRIPTOR_HANDLE cpuDescriptorHandle = pDSVDescriptorHeapManager->GetCPUHandle(m_allocationRTV_DSV);
				pDevice->CreateDepthStencilView(m_pTexture.Get(), &dsvDesc, cpuDescriptorHandle);
			}
			else
			{
				D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
				rtvDesc.Format = dxgiFormat;
				rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
				rtvDesc.Texture2D.MipSlice = 0;
				rtvDesc.Texture2D.PlaneSlice = 0;

				WDescriptorHeapManager* pRTVDescriptorHeapManager = WaveManager::Instance()->GetRTVHeap();
				m_allocationRTV_DSV = pRTVDescriptorHeapManager->Allocate();
				D3D12_CPU_DESCRIPTOR_HANDLE cpuDescriptorHandle = pRTVDescriptorHeapManager->GetCPUHandle(m_allocationRTV_DSV);
				pDevice->CreateRenderTargetView(m_pTexture.Get(), &rtvDesc, cpuDescriptorHandle);
			}
		}
		if (rDescriptor.usage & WTextureDescriptor::Usage::ShaderResource)
		{
			WDescriptorHeapManager* pCBVDescriptorHeapManager = WaveManager::Instance()->GetCBV_SRV_UAVHeap();
			if (m_doesOwnAllocationSRV)
			{
				// Allocate CPU descriptor handle for CBV/SRV/UAV based on buffer type
				m_allocationSRV = pCBVDescriptorHeapManager->Allocate();
			}
			D3D12_CPU_DESCRIPTOR_HANDLE cpuDescriptorHandle = pCBVDescriptorHeapManager->GetCPUHandle(m_allocationSRV.index + m_offsetSRV);
			
			if (m_isDepthType)
			{
				D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
				srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
				srvDesc.Format = dxgiFormatNonTypelessForDepthSRV;
				srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
				srvDesc.Texture2D.MostDetailedMip = 0;
				srvDesc.Texture2D.MipLevels = 1;
				srvDesc.Texture2D.PlaneSlice = 0;
				srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;

				pDevice->CreateShaderResourceView(m_pTexture.Get(), &srvDesc, cpuDescriptorHandle);
			}
			else
			{
				D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
				srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
				srvDesc.Format = dxgiFormat;
				srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
				srvDesc.Texture2D.MostDetailedMip = 0;
				srvDesc.Texture2D.MipLevels = 1;
				srvDesc.Texture2D.PlaneSlice = 0;
				srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;

				pDevice->CreateShaderResourceView(m_pTexture.Get(), &srvDesc, cpuDescriptorHandle);
			}
		}

		if (rDescriptor.pInitalData)
		{
			UploadData(rDescriptor.pInitalData, initialState, finalState);
		}
	}

	WTexture::~WTexture()
	{
		if (m_doesOwnAllocationSRV)
		{
			if (!WDescriptorHeapManager::IsInvalidAllocation(m_allocationSRV))
			{
				WDescriptorHeapManager* pCBVDescriptorHeapManager = WaveManager::Instance()->GetCBV_SRV_UAVHeap();
				pCBVDescriptorHeapManager->Deallocate(m_allocationSRV);
			}
		}

		if (!WDescriptorHeapManager::IsInvalidAllocation(m_allocationRTV_DSV))
		{
			if (m_isDepthType)
			{
				WDescriptorHeapManager* pDSVDescriptorHeapManager = WaveManager::Instance()->GetDSVHeap();
				pDSVDescriptorHeapManager->Deallocate(m_allocationRTV_DSV);
			}
			else
			{
				WDescriptorHeapManager* pRTVDescriptorHeapManager = WaveManager::Instance()->GetRTVHeap();
				pRTVDescriptorHeapManager->Deallocate(m_allocationRTV_DSV);
			}
		}
	}

	void WTexture::UploadData(const void* pData)
	{
		D3D12_RESOURCE_STATES currentState = m_currentState == State::Input ? m_shaderResourceState : m_renderTargetState;
		UploadData(pData, currentState, currentState);
	}

	void WTexture::UploadData(const void* pData, D3D12_RESOURCE_STATES currentState, D3D12_RESOURCE_STATES finalState)
	{
		WaveManager::Instance()->GetUploadManager()->UploadDataToTexture(m_pTexture.Get(), pData, m_bytesPerPixel, currentState, finalState);
	}

	bool WTexture::SetState(State state)
	{
		if (m_currentState == state)
		{
			return false;
		}

		if (m_renderTargetState != m_shaderResourceState)
		{
			WaveECommandList* pCommandList = WaveManager::Instance()->GetCommandList();

			D3D12_RESOURCE_BARRIER barrier = {};
			barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
			barrier.Transition.pResource = GetTexture();
			barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
			barrier.Transition.StateBefore = m_currentState == State::Input ? m_shaderResourceState : m_renderTargetState;
			barrier.Transition.StateAfter = m_currentState == State::Input ? m_renderTargetState : m_shaderResourceState;

			pCommandList->ResourceBarrier(1, &barrier);
		}

		m_currentState = state;

		return true;
	}

	D3D12_CPU_DESCRIPTOR_HANDLE WTexture::GetCPUDescriptorHandle() const
	{
		if (m_currentState == State::Input)
		{
			WDescriptorHeapManager* pCBVDescriptorHeapManager = WaveManager::Instance()->GetCBV_SRV_UAVHeap();
			return pCBVDescriptorHeapManager->GetCPUHandle(m_allocationSRV.index + m_offsetSRV);
		}
		else
		{
			if (m_isDepthType)
			{
				WDescriptorHeapManager* pDSVDescriptorHeapManager = WaveManager::Instance()->GetDSVHeap();
				return pDSVDescriptorHeapManager->GetCPUHandle(m_allocationRTV_DSV);
			}
			else
			{
				WDescriptorHeapManager* pRTVDescriptorHeapManager = WaveManager::Instance()->GetRTVHeap();
				return pRTVDescriptorHeapManager->GetCPUHandle(m_allocationRTV_DSV);
			}
		}
	}

	D3D12_RESOURCE_STATES WTexture::GetCurrentState() const
	{
		return m_currentState == Input ? m_shaderResourceState : m_renderTargetState;
	}
}