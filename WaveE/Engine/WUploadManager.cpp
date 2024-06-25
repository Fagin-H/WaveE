#include "stdafx.h"
#include "WUploadManager.h"
#include "WaveManager.h"

namespace WaveE
{
	WUploadManager::~WUploadManager()
	{
		
	}

	void WUploadManager::Init(size_t bufferSize, size_t bufferCount)
	{
		m_bufferSize = bufferSize;

		for (UINT i = 0; i < bufferCount; ++i)
		{
			CreateUploadBuffer();
		}
	}

	void WUploadManager::UploadDataToBuffer(ID3D12Resource* pDestResource, const void* pData, size_t size, D3D12_RESOURCE_STATES currentState, D3D12_RESOURCE_STATES finalState)
	{
		WAVEE_ASSERT_MESSAGE(size <= m_bufferSize, "Data too big for upload buffer!");

		WaveECommandList* pCommandList = WaveManager::Instance()->GetCommandList();
		WaveECommandQueue* pCommandQueue = WaveManager::Instance()->GetCommandQueue();

		UINT bufferIndex = RequestUploadBuffer();

		void* pMappedData = nullptr;
		D3D12_RANGE readRange = { 0, 0 }; // We do not intend to read this resource on the CPU
		HRESULT hr = m_vUploadBuffers[bufferIndex].pResource->Map(0, &readRange, &pMappedData);
		WAVEE_ASSERT_MESSAGE(SUCCEEDED(hr), "Failed to map upload buffer!");

		memcpy(pMappedData, pData, size);
		m_vUploadBuffers[bufferIndex].pResource->Unmap(0, nullptr);

		if (currentState != D3D12_RESOURCE_STATE_COPY_DEST)
		{
			// Transition the resource to the copy destination state
			CD3DX12_RESOURCE_BARRIER barrierBeforeCopy = CD3DX12_RESOURCE_BARRIER::Transition(pDestResource, currentState, D3D12_RESOURCE_STATE_COPY_DEST);
			pCommandList->ResourceBarrier(1, &barrierBeforeCopy);
		}

		// Copy the data to the destination resource
		pCommandList->CopyBufferRegion(pDestResource, 0, m_vUploadBuffers[bufferIndex].pResource.Get(), 0, size);

		if (finalState != D3D12_RESOURCE_STATE_COPY_DEST)
		{
			// Transition the resource to the final state
			CD3DX12_RESOURCE_BARRIER barrierAfterCopy = CD3DX12_RESOURCE_BARRIER::Transition(pDestResource, D3D12_RESOURCE_STATE_COPY_DEST, finalState);
			pCommandList->ResourceBarrier(1, &barrierAfterCopy);
		}
	}

	void WUploadManager::UploadDataToTexture(ID3D12Resource* pDestResource, const void* pData, UINT bytesPerPixel, D3D12_RESOURCE_STATES currentState, D3D12_RESOURCE_STATES finalState)
	{
		WaveECommandList* pCommandList = WaveManager::Instance()->GetCommandList();
		WaveECommandQueue* pCommandQueue = WaveManager::Instance()->GetCommandQueue();
		UINT bufferIndex = RequestUploadBuffer();

		D3D12_RESOURCE_DESC textureDescriptor = pDestResource->GetDesc();

		// Calculate required size
		UINT64 rowPitch = align_value(textureDescriptor.Width * bytesPerPixel, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT);
		UINT64 totalSize = rowPitch * textureDescriptor.Height;

		WAVEE_ASSERT_MESSAGE(totalSize <= m_bufferSize, "Texture data too big for upload buffer!");

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
			CD3DX12_RESOURCE_BARRIER barrierBeforeCopy = CD3DX12_RESOURCE_BARRIER::Transition(
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
			CD3DX12_RESOURCE_BARRIER barrierAfterCopy = CD3DX12_RESOURCE_BARRIER::Transition(
				pDestResource, D3D12_RESOURCE_STATE_COPY_DEST, finalState);
			pCommandList->ResourceBarrier(1, &barrierAfterCopy);
		}
	}

	void WUploadManager::EndFrame()
	{
		for (int i = 0; i < m_vInUseBuffers.size(); )
		{
			UINT bufferIndex = m_vInUseBuffers[i];
			ReleaseUploadBuffer(bufferIndex);
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

		m_vUploadBuffers.push_back(std::move(newBuffer));
		UINT newBefferIndex = static_cast<UINT>(m_vUploadBuffers.size()) - 1;
		m_qAvailableBuffers.push(newBefferIndex);
		return newBefferIndex;
	}
}