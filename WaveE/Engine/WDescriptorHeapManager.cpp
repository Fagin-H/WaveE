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

		m_heapIndices.reserve(m_numDescriptors);
		for (UINT i = 0; i < m_numDescriptors; ++i)
		{
			m_heapIndices.push_back(HeapIndex{ i, true });
		}
	}

	WDescriptorHeapManager::~WDescriptorHeapManager()
	{
		if (m_pDescriptorHeap)
		{
			m_pDescriptorHeap->Release();
		}
	}

	WDescriptorHeapManager::Allocation WDescriptorHeapManager::Allocate(UINT size)
	{
		// #TODO Find a better way to do this that does not fragment the heap
		UINT consecutiveCount = 0;
		for (UINT i = 0; i < m_heapIndices.size(); i++)
		{
			consecutiveCount++;
			if (!m_heapIndices[i].isFree)
			{
				consecutiveCount = 0;
			}

			if (consecutiveCount == size)
			{
				UINT startIndex = i - consecutiveCount + 1;

				for (UINT j = startIndex; j < startIndex + size; j++)
				{
					m_heapIndices[j].isFree = false;
				}

				return Allocation{ startIndex, size };
			}
		}

		WAVEE_ASSERT_MESSAGE(false, "Could not allocate from heap!");

		return InvalidAllocation();
	}

	void WDescriptorHeapManager::Deallocate(Allocation allocation)
	{
		for (UINT i = allocation.index; i < allocation.index + allocation.size; i++)
		{
			WAVEE_ASSERT_MESSAGE(!m_heapIndices[i].isFree, "Deallocating heap index that is already free!");
			m_heapIndices[i].isFree = true;
		}
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