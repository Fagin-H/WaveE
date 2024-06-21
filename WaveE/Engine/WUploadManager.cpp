#include "stdafx.h"
#include "WUploadManager.h"
#include "WaveManager.h"

namespace WaveE
{

	WUploadManager::WUploadManager(size_t bufferSize, size_t bufferCount)
		: m_bufferSize(bufferSize)
	{
		m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
		WAVEE_ASSERT_MESSAGE(m_fenceEvent, "Failed to create fence event!");

		for (UINT i = 0; i < bufferCount; ++i)
		{
			CreateUploadBuffer();
		}
	}

	WUploadManager::~WUploadManager()
	{
		for (auto& buffer : m_vUploadBuffers)
		{
			WaitForUpload(buffer.fenceValue);
		}
		CloseHandle(m_fenceEvent);
	}

	void WUploadManager::UploadData(ID3D12Resource* destResource, const void* data, size_t size, D3D12_RESOURCE_STATES currentState, D3D12_RESOURCE_STATES finalState)
	{
		WAVEE_ASSERT_MESSAGE(size <= m_bufferSize, "Data too big for upload buffer!");

		WaveECommandList* pCommandList = WaveManager::Instance()->GetCommandList();
		WaveECommandQueue* pCommandQueue = WaveManager::Instance()->GetCommandQueue();

		UINT bufferIndex = RequestUploadBuffer();

		void* pMappedData = nullptr;
		D3D12_RANGE readRange = { 0, 0 }; // We do not intend to read this resource on the CPU
		HRESULT hr = m_vUploadBuffers[bufferIndex].pResource->Map(0, &readRange, &pMappedData);
		WAVEE_ASSERT_MESSAGE(SUCCEEDED(hr), "Failed to map upload buffer!");

		memcpy(pMappedData, data, size);
		m_vUploadBuffers[bufferIndex].pResource->Unmap(0, nullptr);

		if (currentState != D3D12_RESOURCE_STATE_COPY_DEST)
		{
			// Transition the resource to the copy destination state
			CD3DX12_RESOURCE_BARRIER barrierBeforeCopy = CD3DX12_RESOURCE_BARRIER::Transition(destResource, currentState, D3D12_RESOURCE_STATE_COPY_DEST);
			pCommandList->ResourceBarrier(1, &barrierBeforeCopy);
		}

		// Copy the data to the destination resource
		pCommandList->CopyBufferRegion(destResource, 0, m_vUploadBuffers[bufferIndex].pResource.Get(), 0, size);

		if (finalState != D3D12_RESOURCE_STATE_COPY_DEST)
		{
			// Transition the resource to the final state
			CD3DX12_RESOURCE_BARRIER barrierAfterCopy = CD3DX12_RESOURCE_BARRIER::Transition(destResource, D3D12_RESOURCE_STATE_COPY_DEST, finalState);
			pCommandList->ResourceBarrier(1, &barrierAfterCopy);
		}

		m_vUploadBuffers[bufferIndex].fenceValue++;
		pCommandQueue->Signal(m_vUploadBuffers[bufferIndex].pFence.Get(), m_vUploadBuffers[bufferIndex].fenceValue);
	}

	void WUploadManager::EndFrame()
	{
		for (int i = 0; i < m_vInUseBuffers.size(); )
		{
			UINT bufferIndex = m_vInUseBuffers[i];
			if (m_vUploadBuffers[bufferIndex].pFence->GetCompletedValue() >= m_vUploadBuffers[bufferIndex].fenceValue)
			{
				ReleaseUploadBuffer(bufferIndex);
			}
			else
			{
				i++;
			}
		}
	}

	UINT WUploadManager::RequestUploadBuffer()
	{
		if (m_qAvailableBuffers.empty())
		{
			return CreateUploadBuffer();
		}

		UINT bufferIndex = m_qAvailableBuffers.front();
		m_qAvailableBuffers.pop();
		m_vInUseBuffers.push_back(bufferIndex);
		return bufferIndex;
	}

	void WUploadManager::ReleaseUploadBuffer(UINT bufferIndex)
	{
		auto it = std::find(m_vInUseBuffers.begin(), m_vInUseBuffers.end(), bufferIndex);
		if (it != m_vInUseBuffers.end())
		{
			m_vInUseBuffers.erase(it);
		}
		else
		{
			WAVEE_ASSERT_MESSAGE(false, "Index not in use!");
		}
		m_qAvailableBuffers.push(bufferIndex);
	}

	UINT WUploadManager::CreateUploadBuffer()
	{
		UploadBuffer newBuffer;

		WaveEDevice* pDevice = WaveManager::Instance()->GetDevice();

		HRESULT hr = pDevice->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(m_bufferSize),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&newBuffer.pResource)
		);
		WAVEE_ASSERT_MESSAGE(SUCCEEDED(hr), "Failed to create upload buffer!");

		hr = pDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&newBuffer.pFence));
		WAVEE_ASSERT_MESSAGE(SUCCEEDED(hr), "Failed to create fence!");

		newBuffer.fenceValue = 0;
		m_vUploadBuffers.push_back(std::move(newBuffer));
		UINT newBefferIndex = static_cast<UINT>(m_vUploadBuffers.size()) - 1;
		m_qAvailableBuffers.push(newBefferIndex);
		return newBefferIndex;
	}

	void WUploadManager::WaitForUpload(UINT64 bufferIndex)
	{
		if (m_vUploadBuffers[bufferIndex].pFence->GetCompletedValue() < m_vUploadBuffers[bufferIndex].fenceValue)
		{
			m_vUploadBuffers[bufferIndex].pFence->SetEventOnCompletion(m_vUploadBuffers[bufferIndex].fenceValue, m_fenceEvent);
			WaitForSingleObject(m_fenceEvent, INFINITE);
		}
	}

}