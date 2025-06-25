#pragma once
#include "WShader.h"
#include "WRootSigniture.h"
#include "WResource.h"

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
		D3D12_BLEND_DESC blendState;
		D3D12_RASTERIZER_DESC rasterizerState;
		D3D12_DEPTH_STENCIL_DESC depthStencilState{};
		D3D12_PRIMITIVE_TOPOLOGY_TYPE topologyType{ D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE };
		UINT numRenderTarget{ 1 };
		DXGI_FORMAT rtvFormats[8]{ DXGI_FORMAT_R8G8B8A8_UNORM };
		DXGI_FORMAT dsvFormat{ DXGI_FORMAT_UNKNOWN };
		UINT sampleCount{ 1 };
	};

	struct WPipelineDescriptorRT
	{
		WRootSigniture* pGlobalRootSignature{ nullptr };

		struct RootSignatureAssociation
		{
			WRootSigniture* pLocalRootSignature{ nullptr };
			std::vector<std::wstring> vSymbols{};
		private:
			std::vector<const WCHAR*> vSymbolPointers;
			D3D12_SUBOBJECT_TO_EXPORTS_ASSOCIATION association{};
			D3D12_LOCAL_ROOT_SIGNATURE localSig{};
			friend class WPipelineRT;
		};
		std::vector<RootSignatureAssociation> vRootSignatureAssociations{};

		struct Library
		{
			ResourceID<WShader> shaderID{};
			std::vector<std::wstring> vExportedSymbols{};
		private:
			std::vector<D3D12_EXPORT_DESC> vExports;
			D3D12_DXIL_LIBRARY_DESC libDesc;
			friend class WPipelineRT;
		};
		std::vector<Library> vLibraries{};

		struct HitGroup
		{
			std::wstring hitGroupName;
			std::wstring closestHitShader;
			std::wstring anyHitShader;
			std::wstring intersectionShader;
		private:
			D3D12_HIT_GROUP_DESC desc = {};
			friend class WPipelineRT;
		};
		std::vector<HitGroup> vHitGroups{};

		UINT maxPayloadSizeInBytes{ 64 };  // Example: 32 for vec3 color + float
		UINT maxAttributeSizeInBytes{ 8 }; // Usually 8 (2 floats for barycentrics)
		UINT maxRecursionDepth{ 1 };
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

	class WPipelineRT
	{
	public:
		WPipelineRT(WPipelineDescriptorRT& rDescriptor);
		~WPipelineRT();

		ID3D12StateObject* GetPipelineStateObject() const { return m_pPipelineStateObject.Get(); }
		ID3D12StateObjectProperties* GetPipelineStateObjectProperties() const { return m_pPipelineStateObjectProperties.Get(); }

	private:
		void BuildShaderExportList(std::vector<std::wstring>& exportedSymbols, const WPipelineDescriptorRT& rDescriptor);

		ComPtr<ID3D12StateObject> m_pPipelineStateObject{ nullptr };
		ComPtr<ID3D12StateObjectProperties> m_pPipelineStateObjectProperties{ nullptr };

		WPipelineDescriptorRT m_rDescriptor;
	};
}

