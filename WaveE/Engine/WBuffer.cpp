#include "stdafx.h"
#include "WBuffer.h"
#include "WaveManager.h"

namespace WaveE
{
	D3D12_RESOURCE_STATES GetResourceState(const WBufferDescriptor& rDescriptor)
	{
		D3D12_RESOURCE_STATES resourceState = D3D12_RESOURCE_STATE_GENERIC_READ;

		if (!rDescriptor.isDynamic)
		{
			switch (rDescriptor.type)
			{
			case WBufferDescriptor::Constant: [[fallthrough]];
			case WBufferDescriptor::Vertex: resourceState = D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER; break;
			case WBufferDescriptor::Index: resourceState = D3D12_RESOURCE_STATE_INDEX_BUFFER; break;
			case WBufferDescriptor::SRV: resourceState = D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE; break;
			case WBufferDescriptor::UAV: resourceState = D3D12_RESOURCE_STATE_UNORDERED_ACCESS; break;
			}
		}

		return resourceState;
	}

	WBuffer::WBuffer(const WBufferDescriptor& rDescriptor, WDescriptorHeapManager::Allocation allocation, UINT offset)
		: m_allocation{ allocation }
		, m_offset{offset}
		, m_doesOwnAllocation{ WDescriptorHeapManager::IsInvalidAllocation(allocation) }
	{
		bool initialData = rDescriptor.pInitalData;
		m_sizeBytes = rDescriptor.m_sizeBytes;
		m_type = rDescriptor.type;
		m_state = GetResourceState(rDescriptor);

		D3D12_HEAP_PROPERTIES heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
		D3D12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(align_value(rDescriptor.m_sizeBytes, 256));

		WaveEDevice* pDevice = WaveManager::Instance()->GetDevice();

		WAVEE_ASSERT_MESSAGE(pDevice, "Failed to get device!");

		HRESULT hr = pDevice->CreateCommittedResource(&heapProperties,
			D3D12_HEAP_FLAG_NONE,
			&resourceDesc,
			m_state,
			nullptr,
			IID_PPV_ARGS(&m_pBuffer));

		WAVEE_ASSERT_MESSAGE(SUCCEEDED(hr), "Failed to create committed resource for buffer!");

		WDescriptorHeapManager* pCBVDescriptorHeapManager = WaveManager::Instance()->GetCBV_SRV_UAVHeap();
		if (m_doesOwnAllocation)
		{
			// Allocate CPU descriptor handle for CBV/SRV/UAV based on buffer type
			m_allocation = pCBVDescriptorHeapManager->Allocate();
		}
		D3D12_CPU_DESCRIPTOR_HANDLE cpuDescriptorHandle = pCBVDescriptorHeapManager->GetCPUHandle(m_allocation.index + m_offset);

		switch (m_type)
		{
		case WBufferDescriptor::Constant: [[fallthrough]];
		case WBufferDescriptor::Vertex:
		{
			D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
			cbvDesc.BufferLocation = m_pBuffer->GetGPUVirtualAddress();
			cbvDesc.SizeInBytes = static_cast<UINT>(align_value(m_sizeBytes, 256));
			pDevice->CreateConstantBufferView(&cbvDesc, cpuDescriptorHandle);
			break;
		}
		// #TODO update WBufferDescriptor to pass in more data to update the creation of views
		case WBufferDescriptor::Index:
		{
			D3D12_SHADER_RESOURCE_VIEW_DESC viewDesc = {};
			viewDesc.Format = DXGI_FORMAT_R32_UINT;
			viewDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
			viewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			viewDesc.Buffer.FirstElement = 0;
			viewDesc.Buffer.NumElements = static_cast<UINT>(m_sizeBytes / sizeof(UINT));
			viewDesc.Buffer.StructureByteStride = 0;
			viewDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
			pDevice->CreateShaderResourceView(m_pBuffer.Get(), &viewDesc, cpuDescriptorHandle);
			break;
		}
		case WBufferDescriptor::SRV:
		{
			D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
			srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
			srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			srvDesc.Buffer.FirstElement = 0;
			srvDesc.Buffer.NumElements = static_cast<UINT>(m_sizeBytes) / sizeof(float);
			srvDesc.Buffer.StructureByteStride = sizeof(float);

			pDevice->CreateShaderResourceView(m_pBuffer.Get(), &srvDesc, cpuDescriptorHandle);
			break;
		}
		case WBufferDescriptor::UAV:
		{
			D3D12_UNORDERED_ACCESS_VIEW_DESC viewDesc = {};
			viewDesc.Format = DXGI_FORMAT_UNKNOWN;
			viewDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
			pDevice->CreateUnorderedAccessView(m_pBuffer.Get(), nullptr, &viewDesc, cpuDescriptorHandle);
			break;
		}
		}


		if (initialData)
		{
			UploadData(rDescriptor.pInitalData, m_sizeBytes);
		}
	}

	WBuffer::~WBuffer()
	{
		if (m_doesOwnAllocation)
		{
			if (!WDescriptorHeapManager::IsInvalidAllocation(m_allocation))
			{
				WDescriptorHeapManager* pCBVDescriptorHeapManager = WaveManager::Instance()->GetCBV_SRV_UAVHeap();
				pCBVDescriptorHeapManager->Deallocate(m_allocation);
			}
		}
	}

	void WBuffer::UploadData(const void* pData, size_t sizeBytes)
	{
		WAVEE_ASSERT_MESSAGE(sizeBytes <= m_sizeBytes, "Data too big for buffer!");

		WaveManager::Instance()->GetUploadManager()->UploadData(m_pBuffer.Get(), pData, sizeBytes, m_state, m_state);
	}

	D3D12_CPU_DESCRIPTOR_HANDLE WBuffer::GetCPUDescriptorHandle() const
	{
		WDescriptorHeapManager* pCBVDescriptorHeapManager = WaveManager::Instance()->GetCBV_SRV_UAVHeap();
		return pCBVDescriptorHeapManager->GetCPUHandle(m_allocation.index + m_offset);
	}

}
