#include "stdafx.h"
#include "WBuffer.h"
#include "WaveManager.h"

namespace WaveE
{
	D3D12_RESOURCE_STATES GetResourceState(const WBufferDescriptor& rDescriptor)
	{
		D3D12_RESOURCE_STATES resourceState = D3D12_RESOURCE_STATE_GENERIC_READ;

		switch (rDescriptor.type)
		{
		case WBufferDescriptor::Constant: [[fallthrough]];
		case WBufferDescriptor::Vertex: resourceState = D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER; break;
		case WBufferDescriptor::Dynamic: resourceState = D3D12_RESOURCE_STATE_GENERIC_READ; break;
		case WBufferDescriptor::Index: resourceState = D3D12_RESOURCE_STATE_INDEX_BUFFER; break;
		case WBufferDescriptor::SRV: resourceState = D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE; break;
		case WBufferDescriptor::UAV: resourceState = D3D12_RESOURCE_STATE_UNORDERED_ACCESS; break;
		}

		return resourceState;
	}

	WBuffer::WBuffer(const WBufferDescriptor& rDescriptor)
	{
		bool initialData = rDescriptor.m_pInitalData;
		m_sizeBytes = rDescriptor.m_sizeBytes;
		m_type = rDescriptor.type;

		D3D12_HEAP_PROPERTIES heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
		/*heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
		heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY::D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL::D3D12_MEMORY_POOL_UNKNOWN;
		heapProperties.CreationNodeMask = 0;
		heapProperties.VisibleNodeMask = 0;*/

		D3D12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(align_value(rDescriptor.m_sizeBytes, 256));
		/*resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		resourceDesc.Alignment = 0;
		resourceDesc.Width = align_value(rDescriptor.m_sizeBytes, 256);
		resourceDesc.Height = 1;
		resourceDesc.DepthOrArraySize = 1;
		resourceDesc.MipLevels = 1;
		resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
		resourceDesc.SampleDesc.Count = 1;
		resourceDesc.SampleDesc.Quality = 0;
		resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;*/

		ID3D12Device1* pDevice = WaveManager::Instance().GetDevice();

		WAVEE_ASSERT(pDevice);

		HRESULT hr = pDevice->CreateCommittedResource(&heapProperties,
			D3D12_HEAP_FLAG_NONE,
			&resourceDesc,
			GetResourceState(rDescriptor),
			nullptr,
			IID_PPV_ARGS(&m_pBuffer));

		WAVEE_ASSERT(SUCCEEDED(hr));

		if (initialData)
		{
			UploadData(rDescriptor.m_pInitalData, m_sizeBytes);
		}
	}

	void WBuffer::UploadData(const void* pData, size_t sizeBytes)
	{
		WAVEE_ASSERT(sizeBytes <= m_sizeBytes);

		void* mappedData = nullptr;
		D3D12_RANGE readRange = { 0, 0 }; // We do not intend to read this resource on the CPU
		HRESULT hr = m_pBuffer->Map(0, &readRange, &mappedData);
		WAVEE_ASSERT(SUCCEEDED(hr));

		memcpy(mappedData, pData, sizeBytes);
		m_pBuffer->Unmap(0, nullptr);
	}
}
