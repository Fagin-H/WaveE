#pragma once
#include "WDescriptorHeapManager.h"

namespace WaveE
{
	struct WTextureDescriptor
	{
		enum Format
		{
			RGBA,
			SRGBA,
			RGBAF16,
			DepthStencilFloat,
			DepthStencilTypeless,
		};

		enum Usage : UINT
		{
			ShaderResource = 0,
			RenderTarget = 1 << 0,
		};

		Format format{ RGBA };
		Usage usage{ ShaderResource };
		bool startAsShaderResource{ true };
		UINT width;
		UINT height;
		const void* pInitalData{ nullptr };
	};

	// A wrapper around a DX12 resource for use as a texture
	class WTexture
	{
	public:
		WTexture(const WTextureDescriptor& rDescriptor, WDescriptorHeapManager::Allocation allocationSRV = WDescriptorHeapManager::InvalidAllocation(), UINT offset = 0);
		~WTexture();

		ID3D12Resource* GetTexture() const { return m_pTexture.Get(); }

		void UploadData(const void* pData);

		enum State
		{
			Input,
			Output,
		};
		
		// Returns true if the state has changed, false otherwise
		bool SetState(State state);

		D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle() const;

		WDescriptorHeapManager::Allocation GetAllocationSRV() const { return m_allocationSRV; }
		WDescriptorHeapManager::Allocation GetAllocationRTV_DSV() const { return m_allocationRTV_DSV; }

		UINT GetWidth() const { return m_width; }
		UINT GetHeight() const { return m_height; }
		D3D12_RESOURCE_STATES GetCurrentState() const;

		bool IsDepthType() const { return m_isDepthType; }

	private:
		size_t m_sizeBytes{ 0 };
		UINT m_width;
		UINT m_height;
		bool m_isDepthType;
		WDescriptorHeapManager::Allocation m_allocationSRV;
		UINT m_offsetSRV;
		bool m_doesOwnAllocationSRV;
		WDescriptorHeapManager::Allocation m_allocationRTV_DSV;
		State m_currentState;
		D3D12_RESOURCE_STATES m_renderTargetState;
		D3D12_RESOURCE_STATES m_shaderResourceState;
		ComPtr<ID3D12Resource> m_pTexture{ nullptr };

		void UploadData(const void* pData, D3D12_RESOURCE_STATES currentState, D3D12_RESOURCE_STATES finalState);
	};
}

