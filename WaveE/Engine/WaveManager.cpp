#include "stdafx.h"
#include "WaveManager.h"
#include <fstream>

namespace WaveE
{
	WAVEE_SINGLETON_CPP(WaveManager);

	WaveManager::WaveManager()
		: m_cbvSrvUavHeap{ D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE, m_descriptorHeapCountCBV_SRV_UAV }
		, m_rtvHeap{ D3D12_DESCRIPTOR_HEAP_TYPE_RTV, D3D12_DESCRIPTOR_HEAP_FLAG_NONE, m_descriptorHeapCountRTV }
		, m_dsvHeap{ D3D12_DESCRIPTOR_HEAP_TYPE_DSV, D3D12_DESCRIPTOR_HEAP_FLAG_NONE, m_descriptorHeapCountDSV }
		, m_sampelerHeap{ D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, D3D12_DESCRIPTOR_HEAP_FLAG_NONE, m_descriptorHeapCountSampler }
		, m_uploadManager{ m_uploadBufferSize, m_uploadBufferCount }
		, m_defaultRootSigniture{}
	{
		// Init all singletons
		WResourceManager::Init();
		WResourceManager::Instance()->LoadShadersFromDirectory(GetShaderDirectory());

		CreateDefaultRootSigniture();
		CreateSlotHLSLIFile();

		WSamplerDescriptor samplerDescriptors[4] = {
			WSamplerDescriptor{ WSamplerDescriptor::Filter::Point, WSamplerDescriptor::AddressMode::Wrap },
			WSamplerDescriptor{ WSamplerDescriptor::Filter::Linear, WSamplerDescriptor::AddressMode::Wrap },
			WSamplerDescriptor{ WSamplerDescriptor::Filter::Point, WSamplerDescriptor::AddressMode::Clamp },
			WSamplerDescriptor{ WSamplerDescriptor::Filter::Linear, WSamplerDescriptor::AddressMode::Clamp }
		};

		m_defaultSamplers = WResourceManager::Instance()->CreateResourceBlock(samplerDescriptors, 4);
	}

	WaveManager::~WaveManager()
	{
		// UnInit all singletons
		WResourceManager::Uninit();
	}

	void WaveManager::BeginFrame()
	{

	}

	void WaveManager::Update()
	{

	}

	void WaveManager::Render()
	{

	}

	void WaveManager::EndFrame()
	{
		m_uploadManager.EndFrame();
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

	void WaveManager::CreateDefaultRootSigniture()
	{
		constexpr UINT numDescriptorTables = 5;
		WRootSigniture::DescriptorTable descriptorTables[numDescriptorTables];
		// Default Samplers
		descriptorTables[0].numSamplers = m_defaultSamplerCount;
		
		// Global
		descriptorTables[1].numCBVs = m_globalCBVCount;
		descriptorTables[1].numSRVs = m_globalSRVCount;
		descriptorTables[2].numSamplers = m_globalSamplerCount;

		// Material
		descriptorTables[3].numCBVs = m_materialCBVCount;
		descriptorTables[3].numSRVs = m_materialSRVCount;
		descriptorTables[4].numSamplers = m_materialSamplerCount;

		WRootSigniture::RootSignatureDescriptor rootSignitureDescriptor;
		rootSignitureDescriptor.descriptorTables = descriptorTables;
		rootSignitureDescriptor.numDescriptorTables = numDescriptorTables;

		m_defaultRootSigniture.CreateRootSigniture(rootSignitureDescriptor);
	}

	void WaveManager::CreateSlotHLSLIFile()
	{
		std::ofstream file("SlotMacros.hlsli");
		WAVEE_ASSERT_MESSAGE(file.is_open(), "Could not create hlsli file");

		// Writing default slots
		for (UINT i = 0; i < m_defaultSamplerCount; ++i)
		{
			file << "#define DEFAULT_SAMPLER_" << i << " (DEFAULT_SAMPLER_SLOT_START + " << i << ")\n";
		}

		// Writing global slots
		for (UINT i = 0; i < m_globalCBVCount; ++i)
		{
			file << "#define GLOBAL_CBV_" << i << " b" << i << "\n";
		}

		for (UINT i = 0; i < m_globalSRVCount; ++i)
		{
			file << "#define GLOBAL_SRV_" << i << " t" << i << "\n";
		}

		for (UINT i = 0; i < m_globalSamplerCount; ++i)
		{
			file << "#define GLOBAL_SAMPLER_" << i << " s" << i + m_defaultSamplerCount << "\n";
		}

		// Writing material slots
		for (UINT i = 0; i < m_materialCBVCount; ++i)
		{
			file << "#define MATERIAL_CBV_" << i << " b" << i + m_globalCBVCount << "\n";
		}

		for (UINT i = 0; i < m_materialSRVCount; ++i)
		{
			file << "#define MATERIAL_SRV_" << i << " t" << i + m_globalSRVCount << "\n";
		}

		for (UINT i = 0; i < m_materialSamplerCount; ++i)
		{
			file << "#define MATERIAL_SAMPLER_" << i << " s" << i + m_globalSamplerCount + m_defaultSamplerCount << "\n";
		}

		file.close();
	}

	std::string WaveManager::GetShaderDirectory()
	{
		char pathBuffer[MAX_PATH];
		GetModuleFileNameA(nullptr, pathBuffer, MAX_PATH);
		std::string pathString{ pathBuffer };
		size_t lastSlashIndex = pathString.find_last_of("\\/");
		pathString = pathString.substr(0, lastSlashIndex);
		pathString += "\\..\\..\\..\\..\\Resources\\Shaders";
		return pathString;
	}

}
