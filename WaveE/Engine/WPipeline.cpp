#include "stdafx.h"
#include "WPipeline.h"
#include "WaveManager.h"

namespace WaveE
{
	WPipelineDescriptor::WPipelineDescriptor()
		: inputLayout{ WaveManager::Instance()->GetDefaultInputLayout() }
		, pRootSigniture{ WaveManager::Instance()->GetDefaultRootSigniture() }
		, blendState{ DefaultBlendDesc() }
		, rasterizerState{ DefaultRasterizerDesc() }
		, depthStencilState{ DefaultDeptStencilDesc() }
	{

	}

	WPipeline::WPipeline(const WPipelineDescriptor& rDescriptor)
	{
		switch (rDescriptor.type)
		{
		case WPipelineDescriptor::Graphics:
			CreateGraphicsPipeline(rDescriptor);
			break;
		case WPipelineDescriptor::Compute:
			CreateComputePipeline(rDescriptor);
			break;
		default:
			WAVEE_ASSERT_MESSAGE(false, "Unknown pipeline type!");
		}
	}

	WPipeline::~WPipeline()
	{

	}

	void WPipeline::CreateGraphicsPipeline(const WPipelineDescriptor& rDescriptor)
	{
		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
		psoDesc.InputLayout = rDescriptor.inputLayout;
		psoDesc.pRootSignature = rDescriptor.pRootSigniture->GetRootSignature();
		psoDesc.VS = rDescriptor.pVertexShader ? rDescriptor.pVertexShader->GetShaderBytecode() : D3D12_SHADER_BYTECODE{};
		psoDesc.PS = rDescriptor.pPixelShader ? rDescriptor.pPixelShader->GetShaderBytecode() : D3D12_SHADER_BYTECODE{};
		psoDesc.BlendState = rDescriptor.blendState;
		psoDesc.RasterizerState = rDescriptor.rasterizerState;
		psoDesc.DepthStencilState = rDescriptor.depthStencilState;
		psoDesc.PrimitiveTopologyType = rDescriptor.topologyType;
		psoDesc.NumRenderTargets = rDescriptor.numRenderTarget;
		for (UINT i = 0; i < rDescriptor.numRenderTarget; ++i)
		{
			psoDesc.RTVFormats[i] = rDescriptor.rtvFormats[i];
		}
		psoDesc.DSVFormat = rDescriptor.dsvFormat;
		psoDesc.SampleDesc.Count = rDescriptor.sampleCount;
		psoDesc.SampleDesc.Quality = 0;
		psoDesc.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;
		psoDesc.NodeMask = 0;
		psoDesc.CachedPSO.pCachedBlob = nullptr;
		psoDesc.CachedPSO.CachedBlobSizeInBytes = 0;
		psoDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
		psoDesc.SampleMask = 0xFFFFFFFF;

		WaveEDevice* pDevice = WaveManager::Instance()->GetDevice();

		HRESULT hr = pDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pPipelineState));

		WAVEE_ASSERT_MESSAGE(SUCCEEDED(hr), "Failed to create graphics pipeline!");
	}

	void WPipeline::CreateComputePipeline(const WPipelineDescriptor& rDescriptor)
	{
		WAVEE_ASSERT_MESSAGE(rDescriptor.pComputeShader, "Compute pipeline must have a compute shader!");
		D3D12_COMPUTE_PIPELINE_STATE_DESC psoDesc = {};
		psoDesc.pRootSignature = rDescriptor.pRootSigniture->GetRootSignature();
		psoDesc.CS = rDescriptor.pComputeShader->GetShaderBytecode();

		WaveEDevice* pDevice = WaveManager::Instance()->GetDevice();

		HRESULT hr = pDevice->CreateComputePipelineState(&psoDesc, IID_PPV_ARGS(&m_pPipelineState));

		WAVEE_ASSERT_MESSAGE(SUCCEEDED(hr), "Failed to create compute pipeline!");
	}
}