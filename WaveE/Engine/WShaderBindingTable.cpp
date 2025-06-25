#include "stdafx.h"
#include "WShaderBindingTable.h"
#include "WaveManager.h"

namespace WaveE
{

	WShaderBindingTable::WShaderBindingTable(const WShaderBindingTableDescriptor& rDescriptor)
	{
		const UINT shaderIdSize = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;

		WaveEDevice* pDevice = WaveManager::Instance()->GetDevice();

		// ----- Calculate sizes and counts -----
		m_rayGenRecordSize = rDescriptor.rayGenRecordSize ? rDescriptor.rayGenRecordSize : align_value(shaderIdSize + rDescriptor.rayGenLocalRootArgs.size(), D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT);
		m_missRecordSize = rDescriptor.missRecordSize ? rDescriptor.missRecordSize : GetMaxRecorldSize(rDescriptor.missLocalRootArgs);
		m_hitGroupRecordSize = rDescriptor.hitGroupRecordSize ? rDescriptor.hitGroupRecordSize : GetMaxRecorldSize(rDescriptor.hitGroupLocalRootArgs);

		m_missCount = static_cast<UINT>(rDescriptor.missExports.size());
		m_hitGroupCount = static_cast<UINT>(rDescriptor.hitGroupExports.size());

		m_missTableSize = m_missRecordSize * m_missCount;
		m_hitTableSize = m_hitGroupRecordSize * m_hitGroupCount;

		m_missStartOffset = align_value(m_rayGenRecordSize, D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT);
		m_hitStartOffset = align_value(m_missStartOffset + m_missTableSize, D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT);

		WBufferDescriptor shaderBindingTableBufferDesc{};
		shaderBindingTableBufferDesc.isDynamic = true;
		shaderBindingTableBufferDesc.isUpload = true;
		shaderBindingTableBufferDesc.sizeBytes = align_value(m_hitStartOffset + m_hitTableSize, D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT);
		shaderBindingTableBufferDesc.type = WBufferDescriptor::Constant;

		m_shaderBindingTableBufferID = WResourceManager::Instance()->CreateResource(shaderBindingTableBufferDesc);

		UINT currentOffset{ 0 };
		// ------------------- RayGen Shader Table -------------------
		{
			const void* shaderID = rDescriptor.pipeline.GetResource()->GetPipelineStateObjectProperties()->GetShaderIdentifier(rDescriptor.rayGenExport.c_str());
			m_shaderBindingTableBufferID.GetResource()->UploadData(shaderID, shaderIdSize, currentOffset);
			
			if (!rDescriptor.rayGenLocalRootArgs.empty())
			{
				m_shaderBindingTableBufferID.GetResource()->UploadData(rDescriptor.rayGenLocalRootArgs.data(), rDescriptor.rayGenLocalRootArgs.size(), currentOffset + shaderIdSize);
			}
			currentOffset += m_rayGenRecordSize;
		}

		// ------------------- Miss Shader Table -------------------
		currentOffset = m_missStartOffset;
		{
			for (UINT i = 0; i < m_missCount; ++i)
			{
				const void* shaderID = rDescriptor.pipeline.GetResource()->GetPipelineStateObjectProperties()->GetShaderIdentifier(rDescriptor.missExports[i].c_str());
				m_shaderBindingTableBufferID.GetResource()->UploadData(shaderID, shaderIdSize, currentOffset);

				if (i < rDescriptor.missLocalRootArgs.size() && !rDescriptor.missLocalRootArgs[i].empty())
				{
					m_shaderBindingTableBufferID.GetResource()->UploadData(rDescriptor.missLocalRootArgs[i].data(), rDescriptor.missLocalRootArgs[i].size(), currentOffset + shaderIdSize);
				}
				currentOffset += m_missRecordSize;
			}
		}

		// ------------------- Hit Group Shader Table -------------------
		currentOffset = m_hitStartOffset;
		{
			for (UINT i = 0; i < m_hitGroupCount; ++i)
			{
				const void* shaderID = rDescriptor.pipeline.GetResource()->GetPipelineStateObjectProperties()->GetShaderIdentifier(rDescriptor.hitGroupExports[i].c_str());
				m_shaderBindingTableBufferID.GetResource()->UploadData(shaderID, shaderIdSize, currentOffset);

				if (i < rDescriptor.hitGroupLocalRootArgs.size() && !rDescriptor.hitGroupLocalRootArgs[i].empty())
				{
					m_shaderBindingTableBufferID.GetResource()->UploadData(rDescriptor.hitGroupLocalRootArgs[i].data(), rDescriptor.hitGroupLocalRootArgs[i].size(), currentOffset + shaderIdSize);
				}
				currentOffset += m_hitGroupRecordSize;
			}
		}
	}

	WShaderBindingTable::~WShaderBindingTable()
	{

	}

	UINT WShaderBindingTable::GetMaxRecorldSize(const std::vector<std::vector<BYTE>>& args)
	{
		const UINT shaderIdSize = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;
		UINT maxSize = 0;
		for (const auto& localRootArgs : args)
		{
			maxSize = std::max<UINT>(maxSize, static_cast<UINT>(localRootArgs.size()));
		}
		return align_value(shaderIdSize + maxSize, D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT);
	}
}