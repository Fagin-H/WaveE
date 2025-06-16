#include "stdafx.h"
#include "WAccelerationStructure.h"
#include "WaveManager.h"

namespace WaveE
{
	ResourceID<WBuffer> WBottomLevelAS::m_scratchBuffer{};

	WBottomLevelAS::WBottomLevelAS(const WBLASDescriptor& rDescriptor)
	{
		const WMesh* pMesh = rDescriptor.meshID.GetResource();
		WAVEE_ASSERT_MESSAGE(pMesh, "Could not get mesh!");
		if (!pMesh)
		{
			return;
		}

		WAVEE_ASSERT_MESSAGE(pMesh->IsTriangleList(), "Unsupported mesh topology");
		if (!pMesh->IsTriangleList())
		{
			return;
		}

		// Create geom desc
		D3D12_RAYTRACING_GEOMETRY_DESC descriptor = {};
		descriptor.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
		descriptor.Triangles.VertexBuffer.StartAddress = pMesh->GetVertexBufferID().GetResource()->GetBuffer()->GetGPUVirtualAddress();
		descriptor.Triangles.VertexBuffer.StrideInBytes = pMesh->GetVertexStride();
		descriptor.Triangles.VertexCount = pMesh->GetVertexCount();
		descriptor.Triangles.VertexFormat = DXGI_FORMAT_R32G32B32_FLOAT;
		descriptor.Triangles.IndexBuffer = pMesh->IsIndexed() ? (pMesh->GetIndexBufferID().GetResource()->GetBuffer()->GetGPUVirtualAddress()) : 0;
		descriptor.Triangles.IndexFormat = pMesh->IsIndexed() ? DXGI_FORMAT_R32_UINT : DXGI_FORMAT_UNKNOWN;
		descriptor.Triangles.IndexCount = pMesh->GetIndexCount();
		descriptor.Triangles.Transform3x4 = 0;
		descriptor.Flags = rDescriptor.flags & WBLASDescriptor::IS_OPAQUE ? D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE : D3D12_RAYTRACING_GEOMETRY_FLAG_NONE;

		// Check size of buffers needed
		D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS prebuildDesc;
		prebuildDesc.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
		prebuildDesc.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
		prebuildDesc.NumDescs = 1;
		prebuildDesc.pGeometryDescs = &descriptor;
		prebuildDesc.Flags = rDescriptor.flags & WBLASDescriptor::ALLOW_UPDATE ? D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_UPDATE : D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_NONE;

		D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO info = {};

		WaveEDevice* pDevice = WaveManager::Instance()->GetDevice();

		pDevice->GetRaytracingAccelerationStructurePrebuildInfo(&prebuildDesc, &info);

		int scratchSizeBytes = align_value(info.ScratchDataSizeInBytes, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);
		int resultSizeBytes = align_value(info.ResultDataMaxSizeInBytes, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);

		// Create scratch buffer
		if (!(m_scratchBuffer.IsValid() && m_scratchBuffer.GetResource()->GetSize() >= scratchSizeBytes))
		{
			WBufferDescriptor scratchBufferDescriptor = {};
			scratchBufferDescriptor.isDynamic = false;
			scratchBufferDescriptor.sizeBytes = scratchSizeBytes;
			scratchBufferDescriptor.type = WBufferDescriptor::RAY_TRACING;

			if (m_scratchBuffer.IsValid())
			{
				// If it already exists but is too small override a new one
				WResourceManager::Instance()->OverrideResource(m_scratchBuffer, scratchBufferDescriptor);
			}
			else
			{
				// Otherwise just create a new buffer
				m_scratchBuffer = WResourceManager::Instance()->CreateResource(scratchBufferDescriptor);
			}
		}

		// Create result buffer
		{
			WBufferDescriptor resultBufferDescriptor = {};
			resultBufferDescriptor.isDynamic = false;
			resultBufferDescriptor.sizeBytes = scratchSizeBytes;
			resultBufferDescriptor.type = WBufferDescriptor::RAY_TRACING;

			m_ASBuffer = WResourceManager::Instance()->CreateResource(resultBufferDescriptor);
		}

		// Create structure
		D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC buildDesc;
		buildDesc.Inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
		buildDesc.Inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
		buildDesc.Inputs.NumDescs = 1;
		buildDesc.Inputs.pGeometryDescs = &descriptor;
		buildDesc.DestAccelerationStructureData = { m_ASBuffer.GetResource()->GetBuffer()->GetGPUVirtualAddress() };
		buildDesc.ScratchAccelerationStructureData = { m_scratchBuffer.GetResource()->GetBuffer()->GetGPUVirtualAddress() };
		buildDesc.SourceAccelerationStructureData = rDescriptor.previousResult.IsValid() ? rDescriptor.previousResult.GetResource()->GetASBuffer().GetResource()->GetBuffer()->GetGPUVirtualAddress() : 0;
		buildDesc.Inputs.Flags = rDescriptor.flags & WBLASDescriptor::ALLOW_UPDATE ? D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_UPDATE : D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_NONE;

		WaveECommandList* pCommandList = WaveManager::Instance()->GetCommandList();

		pCommandList->BuildRaytracingAccelerationStructure(&buildDesc, 0, nullptr);

		// Wait for the builder to complete by setting a barrier on the resulting
		// buffer. This is particularly important as the construction of the top-level
		// hierarchy may be called right afterwards, before executing the command
		// list.
		D3D12_RESOURCE_BARRIER uavBarrier;
		uavBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
		uavBarrier.UAV.pResource = m_ASBuffer.GetResource()->GetBuffer();
		uavBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		pCommandList->ResourceBarrier(1, &uavBarrier);
	}

	WBottomLevelAS::~WBottomLevelAS()
	{

	}

	ResourceID<WaveE::WBuffer> WTopLevelAS::m_scratchBuffer;
	
	WTopLevelAS::WTopLevelAS(const WTLASDescriptor& rDescriptor)
	{
		D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS flags = rDescriptor.previousResult.IsValid() ? D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_UPDATE : D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_NONE;

		D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS prebuildDesc = {};
		prebuildDesc.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;
		prebuildDesc.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
		prebuildDesc.NumDescs = rDescriptor.numTLASInstanceDescriptors;
		prebuildDesc.Flags = flags;

		D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO info = {};

		WaveEDevice* pDevice = WaveManager::Instance()->GetDevice();

		pDevice->GetRaytracingAccelerationStructurePrebuildInfo(&prebuildDesc, &info);

		int scratchSizeBytes = align_value(info.ScratchDataSizeInBytes, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);
		int resultSizeBytes = align_value(info.ResultDataMaxSizeInBytes, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);

		int instanceDescsSizeInBytes = align_value(sizeof(D3D12_RAYTRACING_INSTANCE_DESC) * rDescriptor.numTLASInstanceDescriptors, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);

		// Create scratch buffer
		if (!(m_scratchBuffer.IsValid() && m_scratchBuffer.GetResource()->GetSize() >= scratchSizeBytes))
		{
			WBufferDescriptor scratchBufferDescriptor = {};
			scratchBufferDescriptor.isDynamic = false;
			scratchBufferDescriptor.sizeBytes = scratchSizeBytes;
			scratchBufferDescriptor.type = WBufferDescriptor::RAY_TRACING;

			if (m_scratchBuffer.IsValid())
			{
				// If it already exists but is too small override a new one
				WResourceManager::Instance()->OverrideResource(m_scratchBuffer, scratchBufferDescriptor);
			}
			else
			{
				// Otherwise just create a new buffer
				m_scratchBuffer = WResourceManager::Instance()->CreateResource(scratchBufferDescriptor);
			}
		}

		// Create Result buffer
		{
			WBufferDescriptor resultBufferDescriptor = {};
			resultBufferDescriptor.isDynamic = false;
			resultBufferDescriptor.sizeBytes = scratchSizeBytes;
			resultBufferDescriptor.type = WBufferDescriptor::RAY_TRACING;

			m_ASBuffer = WResourceManager::Instance()->CreateResource(resultBufferDescriptor);
		}

		// Create descriptor buffer
		{
			WBufferDescriptor descriptoBufferDescriptor = {};
			descriptoBufferDescriptor.isUpload = true;
			descriptoBufferDescriptor.isDynamic = true;
			descriptoBufferDescriptor.sizeBytes = instanceDescsSizeInBytes;
			descriptoBufferDescriptor.type = WBufferDescriptor::DESCRIPTOR;

			m_descriptorBuffer = WResourceManager::Instance()->CreateResource(descriptoBufferDescriptor);
		}

		// Copy the descriptors in the target descriptor buffer
		D3D12_RAYTRACING_INSTANCE_DESC* pInstanceDescs;
		m_descriptorBuffer.GetResource()->GetBuffer()->Map(0, nullptr, reinterpret_cast<void**>(&pInstanceDescs));
		WAVEE_ASSERT_MESSAGE(pInstanceDescs, "Could not map instance descs!");

		auto instanceCount = rDescriptor.numTLASInstanceDescriptors;

		// Create the description for each instance
		for (uint32_t i = 0; i < instanceCount; i++)
		{
			pInstanceDescs[i].InstanceID = rDescriptor.pTLASInstanceDescriptors[i].instanceID;
			pInstanceDescs[i].InstanceContributionToHitGroupIndex = rDescriptor.pTLASInstanceDescriptors[i].hitGroup;
			pInstanceDescs[i].Flags = D3D12_RAYTRACING_INSTANCE_FLAG_NONE;

			memcpy(pInstanceDescs[i].Transform, &rDescriptor.pTLASInstanceDescriptors[i].transform, sizeof(pInstanceDescs[i].Transform));
			pInstanceDescs[i].AccelerationStructure = rDescriptor.pTLASInstanceDescriptors[i].BLAS.GetResource()->GetASBuffer().GetResource()->GetBuffer()->GetGPUVirtualAddress();
			pInstanceDescs[i].InstanceMask = 0xFF;
		}

		m_descriptorBuffer.GetResource()->GetBuffer()->Unmap(0, nullptr);

		// If this in an update operation we need to provide the source buffer
		D3D12_GPU_VIRTUAL_ADDRESS pSourceAS = rDescriptor.previousResult.IsValid() ? rDescriptor.previousResult.GetResource()->GetASBuffer().GetResource()->GetBuffer()->GetGPUVirtualAddress() : 0;

		D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC buildDesc = {};
		buildDesc.Inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;
		buildDesc.Inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
		buildDesc.Inputs.InstanceDescs = m_descriptorBuffer.GetResource()->GetBuffer()->GetGPUVirtualAddress();
		buildDesc.Inputs.NumDescs = instanceCount;
		buildDesc.DestAccelerationStructureData = { m_ASBuffer.GetResource()->GetBuffer()->GetGPUVirtualAddress() };
		buildDesc.ScratchAccelerationStructureData = { m_scratchBuffer.GetResource()->GetBuffer()->GetGPUVirtualAddress() };
		buildDesc.SourceAccelerationStructureData = pSourceAS;
		buildDesc.Inputs.Flags = flags;

		WaveECommandList* pCommandList = WaveManager::Instance()->GetCommandList();

		pCommandList->BuildRaytracingAccelerationStructure(&buildDesc, 0, nullptr);

		// Wait for the builder to complete by setting a barrier on the resulting
		// buffer. This can be important in case the rendering is triggered
		// immediately afterwards, without executing the command list
		D3D12_RESOURCE_BARRIER uavBarrier;
		uavBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
		uavBarrier.UAV.pResource = m_ASBuffer.GetResource()->GetBuffer();
		uavBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		pCommandList->ResourceBarrier(1, &uavBarrier);
	}

	WTopLevelAS::~WTopLevelAS()
	{

	}
}