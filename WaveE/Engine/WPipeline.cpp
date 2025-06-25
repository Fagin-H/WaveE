#include "stdafx.h"
#include "WPipeline.h"
#include "WaveManager.h"
#include <unordered_set>

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

	WPipelineRT::WPipelineRT(WPipelineDescriptorRT& rDescriptor)
	{
		m_rDescriptor = rDescriptor;
		// The pipeline is made of a set of sub-objects, representing the DXIL libraries, hit group
		// declarations, root signature associations, plus some configuration objects
		UINT64 subobjectCount =
			m_rDescriptor.vLibraries.size() +                     // DXIL libraries
			m_rDescriptor.vHitGroups.size() +                     // Hit group declarations
			1 +													// Shader configuration
			//1 +													// Shader payload
			2 * m_rDescriptor.vRootSignatureAssociations.size() + // Root signature declaration + association
			1 +													// Global root signature
			1;													// Final pipeline subobject

		// Initialize a vector with the target object count. It is necessary to make the allocation before
		// adding subobjects as some subobjects reference other subobjects by pointer. Using push_back may
		// reallocate the array and invalidate those pointers.
		std::vector<D3D12_STATE_SUBOBJECT> subobjects(subobjectCount);

		UINT currentIndex = 0;

		// Add all the DXIL libraries
		for (WPipelineDescriptorRT::Library& lib : m_rDescriptor.vLibraries)
		{
			// Create one export descriptor per symbol
			lib.vExports.clear();
			lib.vExports.resize(lib.vExportedSymbols.size());
			for (size_t i = 0; i < lib.vExportedSymbols.size(); i++)
			{
				D3D12_EXPORT_DESC exportDesc{};
				exportDesc.Name = lib.vExportedSymbols[i].c_str();
				exportDesc.ExportToRename = nullptr;
				exportDesc.Flags = D3D12_EXPORT_FLAG_NONE;
				lib.vExports[i] = exportDesc;
			}

			// Create a library descriptor combining the DXIL code and the export names
			WAVEE_ASSERT_MESSAGE(lib.shaderID.IsValid(), "Libraries need shader!");
			lib.libDesc.DXILLibrary.BytecodeLength = lib.shaderID.GetResource()->GetShaderBytecode().BytecodeLength;
			lib.libDesc.DXILLibrary.pShaderBytecode = lib.shaderID.GetResource()->GetShaderBytecode().pShaderBytecode;
			lib.libDesc.NumExports = static_cast<UINT>(lib.vExports.size());
			lib.libDesc.pExports = lib.vExports.data();

			D3D12_STATE_SUBOBJECT libSubobject = {};
			libSubobject.Type = D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY;
			libSubobject.pDesc = &lib.libDesc;

			subobjects[currentIndex++] = libSubobject;
		}

		// Add all the hit group declarations
		for (WPipelineDescriptorRT::HitGroup& group : m_rDescriptor.vHitGroups)
		{
			group.desc.HitGroupExport = group.hitGroupName.c_str();
			group.desc.ClosestHitShaderImport = group.closestHitShader.empty() ? nullptr : group.closestHitShader.c_str();
			group.desc.AnyHitShaderImport = group.anyHitShader.empty() ? nullptr : group.anyHitShader.c_str();
			group.desc.IntersectionShaderImport = group.intersectionShader.empty() ? nullptr : group.intersectionShader.c_str();

			D3D12_STATE_SUBOBJECT hitGroup = {};
			hitGroup.Type = D3D12_STATE_SUBOBJECT_TYPE_HIT_GROUP;
			hitGroup.pDesc = &group.desc;

			subobjects[currentIndex++] = hitGroup;
		}

		// Add a subobject for the shader payload configuration
		D3D12_RAYTRACING_SHADER_CONFIG shaderDesc = {};
		shaderDesc.MaxPayloadSizeInBytes = m_rDescriptor.maxPayloadSizeInBytes;
		shaderDesc.MaxAttributeSizeInBytes = m_rDescriptor.maxAttributeSizeInBytes;

		D3D12_STATE_SUBOBJECT shaderConfigObject = {};
		shaderConfigObject.Type = D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_SHADER_CONFIG;
		shaderConfigObject.pDesc = &shaderDesc;

		subobjects[currentIndex++] = shaderConfigObject;

		//// Build a list of all the symbols for ray generation, miss and hit groups
		//// Those shaders have to be associated with the payload definition
		//std::vector<std::wstring> exportedSymbols = {};
		//std::vector<LPCWSTR> exportedSymbolPointers = {};
		//BuildShaderExportList(exportedSymbols, m_rDescriptor);

		//// Build an array of the string pointers
		//exportedSymbolPointers.reserve(exportedSymbols.size());
		//for (const auto& name : exportedSymbols)
		//{
		//	exportedSymbolPointers.push_back(name.c_str());
		//}
		//const WCHAR** shaderExports = exportedSymbolPointers.data();

		//// Add a subobject for the association between shaders and the payload
		//D3D12_SUBOBJECT_TO_EXPORTS_ASSOCIATION shaderPayloadAssociation = {};
		//shaderPayloadAssociation.NumExports = static_cast<UINT>(exportedSymbols.size());
		//shaderPayloadAssociation.pExports = shaderExports;

		//// Associate the set of shaders with the payload defined in the previous subobject
		//shaderPayloadAssociation.pSubobjectToAssociate = &subobjects[(currentIndex - 1)];

		//// Create and store the payload association object
		//D3D12_STATE_SUBOBJECT shaderPayloadAssociationObject = {};
		//shaderPayloadAssociationObject.Type = D3D12_STATE_SUBOBJECT_TYPE_SUBOBJECT_TO_EXPORTS_ASSOCIATION;
		//shaderPayloadAssociationObject.pDesc = &shaderPayloadAssociation;
		//subobjects[currentIndex++] = shaderPayloadAssociationObject;

		// The root signature association requires two objects for each: one to declare the root
		// signature, and another to associate that root signature to a set of symbols
		//for (WPipelineDescriptorRT::RootSignatureAssociation& assoc : m_rDescriptor.vRootSignatureAssociations)
		//{
		//	assoc.vSymbolPointers.clear();
		//	assoc.vSymbolPointers.resize(assoc.vSymbols.size());
		//	for (size_t i = 0; i < assoc.vSymbols.size(); i++)
		//	{
		//		assoc.vSymbolPointers[i] = assoc.vSymbols[i].c_str();
		//	}

		//	// Add a subobject to declare the root signature
		//	assoc.localSig.pLocalRootSignature = assoc.pLocalRootSignature->GetRootSignature();

		//	D3D12_STATE_SUBOBJECT rootSigObject = {};
		//	rootSigObject.Type = D3D12_STATE_SUBOBJECT_TYPE_LOCAL_ROOT_SIGNATURE;
		//	rootSigObject.pDesc = &assoc.localSig;

		//	subobjects[currentIndex++] = rootSigObject;

		//	// Add a subobject for the association between the exported shader symbols and the root
		//	// signature
		//	assoc.association.NumExports = static_cast<UINT>(assoc.vSymbolPointers.size());
		//	assoc.association.pExports = assoc.vSymbolPointers.data();
		//	assoc.association.pSubobjectToAssociate = &subobjects[(currentIndex - 1)];

		//	D3D12_STATE_SUBOBJECT rootSigAssociationObject = {};
		//	rootSigAssociationObject.Type = D3D12_STATE_SUBOBJECT_TYPE_SUBOBJECT_TO_EXPORTS_ASSOCIATION;
		//	rootSigAssociationObject.pDesc = &assoc.association;

		//	subobjects[currentIndex++] = rootSigAssociationObject;
		//}

		D3D12_GLOBAL_ROOT_SIGNATURE globalSig{};
		globalSig.pGlobalRootSignature = rDescriptor.pGlobalRootSignature->GetRootSignature();

		D3D12_STATE_SUBOBJECT globalRootSig;
		globalRootSig.Type = D3D12_STATE_SUBOBJECT_TYPE_GLOBAL_ROOT_SIGNATURE;
		globalRootSig.pDesc = &globalSig;

		subobjects[currentIndex++] = globalRootSig;

		// Add a subobject for the ray tracing pipeline configuration
		D3D12_RAYTRACING_PIPELINE_CONFIG pipelineConfig = {};
		pipelineConfig.MaxTraceRecursionDepth = m_rDescriptor.maxRecursionDepth;

		D3D12_STATE_SUBOBJECT pipelineConfigObject = {};
		pipelineConfigObject.Type = D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_PIPELINE_CONFIG;
		pipelineConfigObject.pDesc = &pipelineConfig;

		subobjects[currentIndex++] = pipelineConfigObject;

		// Describe the ray tracing pipeline state object
		D3D12_STATE_OBJECT_DESC pipelineDesc = {};
		pipelineDesc.Type = D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE;
		pipelineDesc.NumSubobjects = currentIndex; // static_cast<UINT>(subobjects.size());
		pipelineDesc.pSubobjects = subobjects.data();

		// Create the state object
		WaveEDevice* pDevice = WaveManager::Instance()->GetDevice();

		HRESULT hr = pDevice->CreateStateObject(&pipelineDesc, IID_PPV_ARGS(&m_pPipelineStateObject));

		WAVEE_ASSERT_MESSAGE(SUCCEEDED(hr), "Failed to create ray tracing pipeline!");

		hr = m_pPipelineStateObject->QueryInterface(IID_PPV_ARGS(&m_pPipelineStateObjectProperties));

		WAVEE_ASSERT_MESSAGE(SUCCEEDED(hr), "Failed to get ray tracing pipeline properties!");

		//D3D12_DXIL_LIBRARY_DESC libGen{};
		//D3D12_DXIL_LIBRARY_DESC libMiss{};
		//D3D12_DXIL_LIBRARY_DESC libHit{};
		//libGen.DXILLibrary.pShaderBytecode = rDescriptor.vLibraries[0].shaderID.GetResource()->GetShaderBytecode().pShaderBytecode;
		//libGen.DXILLibrary.BytecodeLength = rDescriptor.vLibraries[0].shaderID.GetResource()->GetShaderBytecode().BytecodeLength;

		//libMiss.DXILLibrary.pShaderBytecode = rDescriptor.vLibraries[1].shaderID.GetResource()->GetShaderBytecode().pShaderBytecode;
		//libMiss.DXILLibrary.BytecodeLength = rDescriptor.vLibraries[1].shaderID.GetResource()->GetShaderBytecode().BytecodeLength;

		//libHit.DXILLibrary.pShaderBytecode = rDescriptor.vLibraries[2].shaderID.GetResource()->GetShaderBytecode().pShaderBytecode;
		//libHit.DXILLibrary.BytecodeLength = rDescriptor.vLibraries[2].shaderID.GetResource()->GetShaderBytecode().BytecodeLength;

		//D3D12_HIT_GROUP_DESC hitGroup{};
		//hitGroup.HitGroupExport = L"HitGroup1";
		//hitGroup.Type = D3D12_HIT_GROUP_TYPE_TRIANGLES;
		//hitGroup.ClosestHitShaderImport = L"ClosestHit";

		//D3D12_RAYTRACING_SHADER_CONFIG shaderCfg;
		//shaderCfg.MaxAttributeSizeInBytes = 8;
		//shaderCfg.MaxPayloadSizeInBytes = 32;

		//D3D12_GLOBAL_ROOT_SIGNATURE globalSig;
		//globalSig.pGlobalRootSignature = rDescriptor.pGlobalRootSignature->GetRootSignature();

		//D3D12_RAYTRACING_PIPELINE_CONFIG pipelineCfg;
		//pipelineCfg.MaxTraceRecursionDepth = 3;

		//D3D12_STATE_SUBOBJECT subobjects2[7];
		//subobjects2[0].Type = D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY;
		//subobjects2[0].pDesc = &libGen;
		//subobjects2[1].Type = D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY;
		//subobjects2[1].pDesc = &libMiss;
		//subobjects2[2].Type = D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY;
		//subobjects2[2].pDesc = &libHit;

		//subobjects2[3].Type = D3D12_STATE_SUBOBJECT_TYPE_HIT_GROUP;
		//subobjects2[3].pDesc = &hitGroup;

		//subobjects2[4].Type = D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_SHADER_CONFIG;
		//subobjects2[4].pDesc = &shaderCfg;

		//subobjects2[5].Type = D3D12_STATE_SUBOBJECT_TYPE_GLOBAL_ROOT_SIGNATURE;
		//subobjects2[5].pDesc = &globalSig;

		//subobjects2[6].Type = D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_PIPELINE_CONFIG;
		//subobjects2[6].pDesc = &pipelineCfg;

		//D3D12_STATE_OBJECT_DESC desc{};
		//desc.NumSubobjects = 7;
		//desc.Type = D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE;
		//desc.pSubobjects = &subobjects2[0];

		//HRESULT hr = pDevice->CreateStateObject(&desc, IID_PPV_ARGS(&m_pPipelineStateObject));
		//WAVEE_ASSERT_MESSAGE(SUCCEEDED(hr), "Failed to create ray tracing pipeline!");

		//hr = m_pPipelineStateObject->QueryInterface(IID_PPV_ARGS(&m_pPipelineStateObjectProperties));
		//WAVEE_ASSERT_MESSAGE(SUCCEEDED(hr), "Failed to get ray tracing pipeline properties!");
	}

	WPipelineRT::~WPipelineRT()
	{

	}

	void WPipelineRT::BuildShaderExportList(std::vector<std::wstring>& exportedSymbols, const WPipelineDescriptorRT& rDescriptor)
	{
		// Get all names from libraries
		// Get names associated to hit groups
		// Return list of libraries+hit group names - shaders in hit groups

		std::unordered_set<std::wstring> exports;

		// Add all the symbols exported by the libraries
		for (const WPipelineDescriptorRT::Library& lib : rDescriptor.vLibraries)
		{
			for (const auto& exportName : lib.vExportedSymbols)
			{
#ifdef _DEBUG
				// Sanity check in debug mode: check that no name is exported more than once
				WAVEE_ASSERT_MESSAGE(exports.find(exportName) == exports.end(), "Symbol exported more than once!");
#endif
				exports.insert(exportName);
			}
		}

#ifdef _DEBUG
		// Sanity check in debug mode: verify that the hit groups do not reference an unknown shader name
		std::unordered_set<std::wstring> all_exports = exports;

		for (const auto& hitGroup : rDescriptor.vHitGroups)
		{
			WAVEE_ASSERT_MESSAGE(!(!hitGroup.anyHitShader.empty() && exports.find(hitGroup.anyHitShader) == exports.end()), "Any hit symbol not found in the imported DXIL libraries");

			WAVEE_ASSERT_MESSAGE(!(!hitGroup.closestHitShader.empty() && exports.find(hitGroup.closestHitShader) == exports.end()), "Closest hit symbol not found in the imported DXIL libraries");

			WAVEE_ASSERT_MESSAGE(!(!hitGroup.intersectionShader.empty() && exports.find(hitGroup.intersectionShader) == exports.end()), "Intersection symbol not found in the imported DXIL libraries");

			all_exports.insert(hitGroup.hitGroupName);
		}

		// Sanity check in debug mode: verify that the root signature associations do not reference an
		// unknown shader or hit group name
		for (const auto& assoc : rDescriptor.vRootSignatureAssociations)
		{
			for (const auto& symb : assoc.vSymbols)
			{
				WAVEE_ASSERT_MESSAGE(!(!symb.empty() && all_exports.find(symb) == all_exports.end()), "Root association symbol not found in the imported DXIL libraries and hit group names");
			}
		}
#endif

		// Go through all hit groups and remove the symbols corresponding to intersection, any hit and
		// closest hit shaders from the symbol set
		for (const auto& hitGroup : rDescriptor.vHitGroups)
		{
			if (!hitGroup.anyHitShader.empty())
			{
				exports.erase(hitGroup.anyHitShader);
			}
			if (!hitGroup.closestHitShader.empty())
			{
				exports.erase(hitGroup.closestHitShader);
			}
			if (!hitGroup.intersectionShader.empty())
			{
				exports.erase(hitGroup.intersectionShader);
			}
			exports.insert(hitGroup.hitGroupName);
		}

		// Finally build a vector containing ray generation and miss shaders, plus the hit group names
		for (const auto& name : exports)
		{
			exportedSymbols.push_back(name);
		}
	}
}