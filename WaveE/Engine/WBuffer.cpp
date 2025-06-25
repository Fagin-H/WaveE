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
			case WBufferDescriptor::RAY_TRACING_AS: resourceState = D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE; break;
			case WBufferDescriptor::RAY_TRACING_VERTEX: resourceState = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE; break;
			}
		}

		return resourceState;
	}

	D3D12_RESOURCE_STATES GetInitialResourceState(const WBufferDescriptor& rDescriptor)
	{
		D3D12_RESOURCE_STATES resourceState = D3D12_RESOURCE_STATE_GENERIC_READ;

		if (!rDescriptor.isDynamic)
		{
			switch (rDescriptor.type)
			{
			case WBufferDescriptor::Constant: [[fallthrough]];
			case WBufferDescriptor::Vertex: resourceState = D3D12_RESOURCE_STATE_COMMON; break;
			case WBufferDescriptor::Index: resourceState = D3D12_RESOURCE_STATE_INDEX_BUFFER; break;
			case WBufferDescriptor::SRV: resourceState = D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE; break;
			case WBufferDescriptor::UAV: resourceState = D3D12_RESOURCE_STATE_UNORDERED_ACCESS; break;
			case WBufferDescriptor::RAY_TRACING_AS: resourceState = D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE; break;
			case WBufferDescriptor::RAY_TRACING_VERTEX: resourceState = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE; break;
			}
		}

		return resourceState;
	}

	D3D12_RESOURCE_FLAGS GetResourceFlags(const WBufferDescriptor& rDescriptor)
	{
		if (rDescriptor.type == WBufferDescriptor::RAY_TRACING_AS || rDescriptor.type == WBufferDescriptor::UAV)
		{
			return D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
		}

		return D3D12_RESOURCE_FLAG_NONE;
	}

	WBuffer::WBuffer(const WBufferDescriptor& rDescriptor, WDescriptorHeapManager::Allocation allocation, UINT offset)
		: m_allocation{ allocation }
		, m_offset{offset}
		, m_bDoesOwnAllocation{ WDescriptorHeapManager::IsInvalidAllocation(allocation) }
		, m_bIsUploadBuffer{ rDescriptor.isUpload }
		, m_UAVFormat{ rDescriptor.UAVFormat }
	{
		bool initialData = rDescriptor.pInitalData;
		m_sizeBytes = rDescriptor.sizeBytes;
		m_strideBytes = rDescriptor.strideBytes;
		m_height = rDescriptor.height;
		m_type = rDescriptor.type;
		m_state = GetResourceState(rDescriptor);
		D3D12_RESOURCE_STATES initialState = GetInitialResourceState(rDescriptor);

		D3D12_HEAP_PROPERTIES heapProperties = CreateHeapProperties(m_bIsUploadBuffer ? D3D12_HEAP_TYPE_UPLOAD : D3D12_HEAP_TYPE_DEFAULT);
		D3D12_RESOURCE_DESC resourceDesc = (m_type == WBufferDescriptor::UAV && m_height > 0) ? CreateUAVTextureResourceDesc(rDescriptor.sizeBytes, m_height) : CreateBufferResourceDesc(align_value(rDescriptor.sizeBytes, 256));
		if (m_type == WBufferDescriptor::UAV && m_height > 0)
		{
			resourceDesc.Format = rDescriptor.UAVFormat;
		}
		resourceDesc.Flags = GetResourceFlags(rDescriptor);

		WaveEDevice* pDevice = WaveManager::Instance()->GetDevice();

		WAVEE_ASSERT_MESSAGE(pDevice, "Failed to get device!");

		HRESULT hr = pDevice->CreateCommittedResource(&heapProperties,
			D3D12_HEAP_FLAG_NONE,
			&resourceDesc,
			initialState,
			nullptr,
			IID_PPV_ARGS(&m_pBuffer));

		WAVEE_ASSERT_MESSAGE(SUCCEEDED(hr), "Failed to create committed resource for buffer!");

		if (m_state != initialState)
		{
			// Transition state to D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER
			WaveECommandList* pCommandList = WaveManager::Instance()->GetCommandList();

			D3D12_RESOURCE_BARRIER barrierTransition = CreateTransitionBarrier(m_pBuffer.Get(), initialState, m_state);
			pCommandList->ResourceBarrier(1, &barrierTransition);
		}

		WDescriptorHeapManager* pCBVDescriptorHeapManager = WaveManager::Instance()->GetCBV_SRV_UAVHeap();
		if (m_bDoesOwnAllocation)
		{
			// Allocate CPU descriptor handle for CBV/SRV/UAV based on buffer type
			m_allocation = pCBVDescriptorHeapManager->Allocate();
		}
		D3D12_CPU_DESCRIPTOR_HANDLE cpuDescriptorHandle = pCBVDescriptorHeapManager->GetCPUHandle(m_allocation.index + m_offset);

		CreateView(m_allocation, m_offset);

		if (initialData)
		{
			UploadData(rDescriptor.pInitalData, m_sizeBytes);
		}
	}

	WBuffer::~WBuffer()
	{
		if (m_bDoesOwnAllocation)
		{
			if (!WDescriptorHeapManager::IsInvalidAllocation(m_allocation))
			{
				WDescriptorHeapManager* pCBVDescriptorHeapManager = WaveManager::Instance()->GetCBV_SRV_UAVHeap();
				pCBVDescriptorHeapManager->Deallocate(m_allocation);
			}
		}
	}

	void WBuffer::UploadData(const void* pData, size_t sizeBytes, UINT offsetBytes)
	{
		if (m_bIsUploadBuffer)
		{
			BYTE* mappedPtr;
			m_pBuffer->Map(0, nullptr, (void**)&mappedPtr);
			memcpy(mappedPtr + offsetBytes, pData, sizeBytes);
			m_pBuffer->Unmap(0, nullptr);
		}
		else
		{
			WAVEE_ASSERT_MESSAGE(sizeBytes <= m_sizeBytes, "Data too big for buffer!");

			WaveManager::Instance()->GetUploadManager()->UploadDataToBuffer(m_pBuffer.Get(), pData, sizeBytes, offsetBytes, m_state, m_state);
		}
	}

	D3D12_CPU_DESCRIPTOR_HANDLE WBuffer::GetCPUDescriptorHandle() const
	{
		WDescriptorHeapManager* pCBVDescriptorHeapManager = WaveManager::Instance()->GetCBV_SRV_UAVHeap();
		return pCBVDescriptorHeapManager->GetCPUHandle(m_allocation.index + m_offset);
	}

	void WBuffer::CreateView(WDescriptorHeapManager::Allocation allocationSRV, UINT offset /*= 0*/)
	{
		WaveEDevice* pDevice = WaveManager::Instance()->GetDevice();
		WDescriptorHeapManager* pCBVDescriptorHeapManager = WaveManager::Instance()->GetCBV_SRV_UAVHeap();

		D3D12_CPU_DESCRIPTOR_HANDLE cpuDescriptorHandle = pCBVDescriptorHeapManager->GetCPUHandle(allocationSRV.index + offset);

		switch (m_type)
		{
			case WBufferDescriptor::Constant:
			{
				D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
				cbvDesc.BufferLocation = m_pBuffer->GetGPUVirtualAddress();
				cbvDesc.SizeInBytes = static_cast<UINT>(align_value(m_sizeBytes, 256));
				pDevice->CreateConstantBufferView(&cbvDesc, cpuDescriptorHandle);
				break;
			}
			case WBufferDescriptor::Vertex:
			{
				if (m_strideBytes > 0)
				{
					D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
					srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
					srvDesc.Format = DXGI_FORMAT_UNKNOWN;
					srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
					srvDesc.Buffer.FirstElement = 0;
					srvDesc.Buffer.NumElements = static_cast<UINT>(m_sizeBytes / m_strideBytes);
					srvDesc.Buffer.StructureByteStride = m_strideBytes; // Size of your vertex structure
					srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

					pDevice->CreateShaderResourceView(m_pBuffer.Get(), &srvDesc, cpuDescriptorHandle);
				}
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
				if (m_height > 0)
				{
					D3D12_UNORDERED_ACCESS_VIEW_DESC viewDesc = {};
					// Assume UAV is used for ray tracing texture
					viewDesc.Format = m_UAVFormat;
					viewDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
					viewDesc.Texture2D.MipSlice = 0;
					viewDesc.Texture2D.PlaneSlice = 0;
					pDevice->CreateUnorderedAccessView(m_pBuffer.Get(), nullptr, &viewDesc, cpuDescriptorHandle);
				}
				break;
			}
			case WBufferDescriptor::RAY_TRACING_AS:
			{
				D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
				srvDesc.ViewDimension = D3D12_SRV_DIMENSION_RAYTRACING_ACCELERATION_STRUCTURE;
				srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
				srvDesc.RaytracingAccelerationStructure.Location = m_pBuffer.Get()->GetGPUVirtualAddress();

				pDevice->CreateShaderResourceView(nullptr, &srvDesc, cpuDescriptorHandle);
				break;
			}
			case WBufferDescriptor::RAY_TRACING_VERTEX:
			{
				m_bHasView = false;
				break;
			}
			case WBufferDescriptor::DESCRIPTOR:
			{
				m_bHasView = false;
				break;
			}
			default:
			{
				m_bHasView = false;
				break;
			}
		}
	}

}
