#include "stdafx.h"
#include "WDescriptorHeapManager.h"
#include "WaveManager.h"

namespace WaveE
{

	WDescriptorHeapManager::WDescriptorHeapManager(D3D12_DESCRIPTOR_HEAP_TYPE type, D3D12_DESCRIPTOR_HEAP_FLAGS flags, UINT numDescriptors)
	{
		WaveEDevice* pDevice = WaveManager::Instance()->GetDevice();

		D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
		heapDesc.NumDescriptors = numDescriptors;
		heapDesc.Type = type;
		heapDesc.Flags = flags;

		HRESULT hr = pDevice->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_pDescriptorHeap));
		WAVEE_ASSERT_MESSAGE(SUCCEEDED(hr), "Failed to create descriptor heap!");

		m_descriptorSize = pDevice->GetDescriptorHandleIncrementSize(type);
		m_numDescriptors = numDescriptors;

		for (UINT i = 0; i < m_numDescriptors; ++i)
		{
			m_freeList.push(i);
		}
	}

	UINT WDescriptorHeapManager::Allocate()
	{
		WAVEE_ASSERT_MESSAGE(!m_freeList.empty(), "Descriptor heap is full!");

		int index = m_freeList.top();
		m_freeList.pop();
		return index;
	}

	void WDescriptorHeapManager::Deallocate(UINT index)
	{
		WAVEE_ASSERT_MESSAGE(index >= 0 && index < m_numDescriptors, "Descriptor index out of range");

		m_freeList.push(index);
	}

	CD3DX12_GPU_DESCRIPTOR_HANDLE WDescriptorHeapManager::GetGPUHandle(UINT index) const
	{
		WAVEE_ASSERT_MESSAGE(index >= 0 && index < m_numDescriptors, "Descriptor index out of range");

		return CD3DX12_GPU_DESCRIPTOR_HANDLE(m_pDescriptorHeap->GetGPUDescriptorHandleForHeapStart(), index, m_descriptorSize);
	}

	CD3DX12_CPU_DESCRIPTOR_HANDLE WDescriptorHeapManager::GetCPUHandle(UINT index) const
	{
		WAVEE_ASSERT_MESSAGE(index >= 0 && index < m_numDescriptors, "Descriptor index out of range");

		return CD3DX12_CPU_DESCRIPTOR_HANDLE(m_pDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), index, m_descriptorSize);
	}
}