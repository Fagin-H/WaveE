#pragma once

namespace WaveE
{
	struct WTextureDescriptor
	{
		enum Format
		{
			RGBA,
			RGBAF16,
			DepthStencilFloat,
			DepthStencilTypeless,
		};

		enum Usage : UINT
		{
			ShaderResource = 0,
			RenderTarget = 1 << 0,
		};

		Format format;
		Usage usage;
		bool startAsShaderResource{ true };
		UINT width;
		UINT height;
		const void* pInitalData{ nullptr };
	};

	// A wrapper around a DX12 resource for use as a buffer
	class WTexture
	{
		WTexture(const WTextureDescriptor& rDescriptor);
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

	private:
		size_t m_sizeBytes{ 0 };
		UINT m_width;
		UINT m_height;
		bool m_isDepthType;
		UINT m_cpuDescriptorHandleIndexSRV;
		UINT m_cpuDescriptorHandleIndexRTV_DSV;
		State m_currentState;
		D3D12_RESOURCE_STATES m_renderTargetState;
		D3D12_RESOURCE_STATES m_shaderResourceState;
		ComPtr<ID3D12Resource> m_pTexture{ nullptr };

		void UploadData(const void* pData, D3D12_RESOURCE_STATES currentState, D3D12_RESOURCE_STATES finalState);
	};
}

