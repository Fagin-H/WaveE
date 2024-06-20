#include "stdafx.h"
#include "WaveManager.h"
#include <fstream>

namespace WaveE
{
	WAVEE_SINGLETON_CPP(WaveManager);

	WaveManager::WaveManager()
		: m_cbvSrvUavHeap{D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE, m_descriptorHeapCount}
		, m_rtvHeap{ D3D12_DESCRIPTOR_HEAP_TYPE_RTV, D3D12_DESCRIPTOR_HEAP_FLAG_NONE, m_descriptorHeapCount}
		, m_dsvHeap{ D3D12_DESCRIPTOR_HEAP_TYPE_DSV, D3D12_DESCRIPTOR_HEAP_FLAG_NONE, m_descriptorHeapCount}
		, m_uploadManager{ m_uploadBufferSize, m_uploadBufferCount }
		, m_defaultRootSigniture{}
	{
		CreateDefaultRootSigniture();
		CreateSlotHLSLIFile();
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

}
