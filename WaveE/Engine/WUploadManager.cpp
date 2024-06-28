#include "stdafx.h"
#include "WUploadManager.h"
#include "WaveManager.h"

namespace WaveE
{
	WUploadManager::~WUploadManager()
	{
		
	}

	void WUploadManager::Init(size_t bigBufferSize, UINT bigBufferCount, size_t smallBufferSize, UINT smallBufferCount)
	{
		m_bigBufferSize = bigBufferSize;
		m_smallBufferSize = smallBufferSize;

		WaveEDevice* pDevice = WaveManager::Instance()->GetDevice();

		// Create a fence for synchronization
		HRESULT hr = pDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_pFence));
		WAVEE_ASSERT_MESSAGE(SUCCEEDED(hr), "Failed to create fence!");

		m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
		WAVEE_ASSERT_MESSAGE(m_fenceEvent, "Failed to create fence event!");

		for (UINT i = 0; i < bigBufferCount; ++i)
		{
			CreateUploadBuffer(true, m_bigBufferSize);
		}
		for (UINT i = 0; i < smallBufferCount; ++i)
		{
			CreateUploadBuffer(true, m_smallBufferSize);
		}
	}

	void WUploadManager::UploadDataToBuffer(ID3D12Resource* pDestResource, const void* pData, size_t size, D3D12_RESOURCE_STATES currentState, D3D12_RESOURCE_STATES finalState)
	{
		WAVEE_ASSERT_MESSAGE(size <= m_bigBufferSize, "Data too big for upload buffer!");

		UINT bufferIndex = RequestUploadBuffer(size);

		WaveECommandList* pCommandList = WaveManager::Instance()->GetCommandList();
		WaveECommandQueue* pCommandQueue = WaveManager::Instance()->GetCommandQueue();

		void* pMappedData = nullptr;
		D3D12_RANGE readRange = { 0, 0 }; // We do not intend to read this resource on the CPU
		HRESULT hr = m_vUploadBuffers[bufferIndex].pResource->Map(0, &readRange, &pMappedData);
		WAVEE_ASSERT_MESSAGE(SUCCEEDED(hr), "Failed to map upload buffer!");

		memcpy(pMappedData, pData, size);
		m_vUploadBuffers[bufferIndex].pResource->Unmap(0, nullptr);

		if (currentState != D3D12_RESOURCE_STATE_COPY_DEST)
		{
			// Transition the resource to the copy destination state
			D3D12_RESOURCE_BARRIER barrierBeforeCopy = CreateTransitionBarrier(pDestResource, currentState, D3D12_RESOURCE_STATE_COPY_DEST);
			pCommandList->ResourceBarrier(1, &barrierBeforeCopy);
		}

		// Copy the data to the destination resource
		pCommandList->CopyBufferRegion(pDestResource, 0, m_vUploadBuffers[bufferIndex].pResource.Get(), 0, size);

		if (finalState != D3D12_RESOURCE_STATE_COPY_DEST)
		{
			// Transition the resource to the final state
			D3D12_RESOURCE_BARRIER barrierAfterCopy = CreateTransitionBarrier(pDestResource, D3D12_RESOURCE_STATE_COPY_DEST, finalState);
			pCommandList->ResourceBarrier(1, &barrierAfterCopy);
		}
	}

	void WUploadManager::UploadDataToTexture(ID3D12Resource* pDestResource, const void* pData, UINT bytesPerPixel, D3D12_RESOURCE_STATES currentState, D3D12_RESOURCE_STATES finalState)
	{
		WaveECommandList* pCommandList = WaveManager::Instance()->GetCommandList();
		WaveECommandQueue* pCommandQueue = WaveManager::Instance()->GetCommandQueue();

		D3D12_RESOURCE_DESC textureDescriptor = pDestResource->GetDesc();

		// Calculate required size
		UINT64 rowPitch = align_value(textureDescriptor.Width * bytesPerPixel, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT);
		UINT64 totalSize = rowPitch * textureDescriptor.Height;

		WAVEE_ASSERT_MESSAGE(totalSize <= m_bigBufferSize, "Texture data too big for upload buffer!");
		
		UINT bufferIndex = RequestUploadBuffer(totalSize);

		// Map the upload buffer
		void* pMappedData = nullptr;
		D3D12_RANGE readRange = { 0, 0 }; // We do not intend to read this resource on the CPU
		HRESULT hr = m_vUploadBuffers[bufferIndex].pResource->Map(0, &readRange, &pMappedData);
		WAVEE_ASSERT_MESSAGE(SUCCEEDED(hr), "Failed to map upload buffer!");

		// Copy data to the upload buffer
		const char* pSrcData = static_cast<const char*>(pData);
		char* pDstData = static_cast<char*>(pMappedData);
		for (UINT y = 0; y < textureDescriptor.Height; ++y)
		{
			memcpy(pDstData, pSrcData, textureDescriptor.Width * bytesPerPixel);
			pSrcData += textureDescriptor.Width * bytesPerPixel;
			pDstData += rowPitch;
		}

		m_vUploadBuffers[bufferIndex].pResource->Unmap(0, nullptr);

		// Transition the resource to the copy destination state if needed
		if (currentState != D3D12_RESOURCE_STATE_COPY_DEST)
		{
			D3D12_RESOURCE_BARRIER barrierBeforeCopy = CreateTransitionBarrier(
				pDestResource, currentState, D3D12_RESOURCE_STATE_COPY_DEST);
			pCommandList->ResourceBarrier(1, &barrierBeforeCopy);
		}

		// Copy the data to the destination texture
		D3D12_TEXTURE_COPY_LOCATION dst = {};
		dst.pResource = pDestResource;
		dst.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
		dst.SubresourceIndex = 0;

		D3D12_TEXTURE_COPY_LOCATION src = {};
		src.pResource = m_vUploadBuffers[bufferIndex].pResource.Get();
		src.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
		src.PlacedFootprint.Offset = 0;
		src.PlacedFootprint.Footprint.Format = textureDescriptor.Format;
		src.PlacedFootprint.Footprint.Width = textureDescriptor.Width;
		src.PlacedFootprint.Footprint.Height = textureDescriptor.Height;
		src.PlacedFootprint.Footprint.Depth = 1;
		src.PlacedFootprint.Footprint.RowPitch = rowPitch;

		pCommandList->CopyTextureRegion(&dst, 0, 0, 0, &src, nullptr);

		// Transition the resource to the final state if needed
		if (finalState != D3D12_RESOURCE_STATE_COPY_DEST)
		{
			D3D12_RESOURCE_BARRIER barrierAfterCopy = CreateTransitionBarrier(
				pDestResource, D3D12_RESOURCE_STATE_COPY_DEST, finalState);
			pCommandList->ResourceBarrier(1, &barrierAfterCopy);
		}
	}

	void WUploadManager::EndFrame()
	{
		std::vector<UINT> bufferIndicesToRelease;
		for (UINT i = 0; i < m_vInUseBuffers.size(); i++)
		{
			UINT bufferIndex = m_vInUseBuffers[i];

			if (m_pFence->GetCompletedValue() >= m_vUploadBuffers[bufferIndex].fenceValue)
			{
				bufferIndicesToRelease.push_back(bufferIndex);
			}
		}
		for (UINT bufferIndex : bufferIndicesToRelease)
		{
			ReleaseUploadBuffer(bufferIndex);
		}
	}

	UINT WUploadManager::RequestUploadBuffer(size_t bufferSize)
	{
		WaveECommandQueue* pCommandQueue = WaveManager::Instance()->GetCommandQueue();

		// Signal and increment the fence value
		const UINT64 fenceToWaitFor = m_fenceValue;
		HRESULT hr = pCommandQueue->Signal(m_pFence.Get(), fenceToWaitFor);
		WAVEE_ASSERT_MESSAGE(SUCCEEDED(hr), "Failed to signal fence!");

		m_fenceValue++;

		UINT bufferIndex{ UINT_MAX };
		
		// Search for small buffers first
		if (bufferSize <= m_smallBufferSize)
		{
			for (UINT i = 0; i < m_qAvailableBuffers.size(); i++)
			{
				UINT newBufferIndex = m_qAvailableBuffers[i];
				if (m_vUploadBuffers[newBufferIndex].bufferSize == m_smallBufferSize)
				{
					bufferIndex = newBufferIndex;
					break;
				}
			}
		}
		// Otherwise search for big buffers
		if (bufferIndex == UINT_MAX)
		{
			for (UINT i = 0; i < m_qAvailableBuffers.size(); i++)
			{
				UINT newBufferIndex = m_qAvailableBuffers[i];
				if (m_vUploadBuffers[newBufferIndex].bufferSize >= bufferSize)
				{
					bufferIndex = newBufferIndex;
					break;
				}
			}
		}
		// Create new buffer if one cannot be found
		if (bufferIndex == UINT_MAX)
		{
			bufferIndex = CreateUploadBuffer(false, bufferSize > m_smallBufferSize ? m_bigBufferSize : m_smallBufferSize);
		}
		else
		{
			auto it = std::find(m_qAvailableBuffers.begin(), m_qAvailableBuffers.end(), bufferIndex);
			WAVEE_ASSERT_MESSAGE(it != m_qAvailableBuffers.end(), "Could not find buffer index in avilable buffers!");
			m_qAvailableBuffers.erase(it);
			m_vInUseBuffers.push_back(bufferIndex);
		}

		m_vUploadBuffers[bufferIndex].fenceValue = fenceToWaitFor;

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
		m_qAvailableBuffers.push_back(bufferIndex);
	}

	UINT WUploadManager::CreateUploadBuffer(bool addToAvailableBuffers, size_t bufferSize)
	{
		UploadBuffer newBuffer;
		newBuffer.bufferSize = bufferSize;

		WaveEDevice* pDevice = WaveManager::Instance()->GetDevice();

		HRESULT hr = pDevice->CreateCommittedResource(
			&CreateHeapProperties(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CreateBufferResourceDesc(bufferSize),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&newBuffer.pResource)
		);
		WAVEE_ASSERT_MESSAGE(SUCCEEDED(hr), "Failed to create upload buffer!");

		m_vUploadBuffers.push_back(std::move(newBuffer));
		UINT newBufferIndex = static_cast<UINT>(m_vUploadBuffers.size()) - 1;
		
		if (addToAvailableBuffers)
		{
			m_qAvailableBuffers.push_back(newBufferIndex);
		}
		else
		{
			m_vInUseBuffers.push_back(newBufferIndex);
		}

		return newBufferIndex;
	}
}