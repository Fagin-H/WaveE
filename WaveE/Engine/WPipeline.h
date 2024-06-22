#pragma once
#include "WShader.h"
#include "WRootSigniture.h"

namespace WaveE
{
	struct WPipelineDescriptor
	{
		WPipelineDescriptor();

		enum PipelineType
		{
			Graphics,
			Compute
		};

		PipelineType type{ Graphics };

		WShader* pVertexShader{ nullptr };
		WShader* pPixelShader{ nullptr };
		WShader* pComputeShader{ nullptr };

		// Graphics pipeline specific
		D3D12_INPUT_LAYOUT_DESC inputLayout;
		WRootSigniture* pRootSigniture;
		D3D12_BLEND_DESC blendState{ CD3DX12_BLEND_DESC(D3D12_DEFAULT) };
		D3D12_RASTERIZER_DESC rasterizerState{ CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT) };
		D3D12_DEPTH_STENCIL_DESC depthStencilState{ CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT) };
		D3D12_PRIMITIVE_TOPOLOGY_TYPE topologyType{ D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE };
		UINT numRenderTarget{ 1 };
		DXGI_FORMAT rtvFormats[8]{ DXGI_FORMAT_R8G8B8A8_UNORM };
		DXGI_FORMAT dsvFormat{ DXGI_FORMAT_D24_UNORM_S8_UINT };
		UINT sampleCount{ 1 };
	};

	class WPipeline
	{
	public:
		WPipeline(const WPipelineDescriptor& rDescriptor);
		~WPipeline();

		ID3D12PipelineState* GetPipelineState() { return m_pPipelineState.Get(); }

	private:
		void CreateGraphicsPipeline(const WPipelineDescriptor& rDescriptor);
		void CreateComputePipeline(const WPipelineDescriptor& rDescriptor);

		ComPtr<ID3D12PipelineState> m_pPipelineState{ nullptr };
	};
}
