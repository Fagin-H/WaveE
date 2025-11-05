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

		WaveEDevice pDevice = WaveManager::Instance()->GetDevice();

		for (UINT i = 0; i < bigBufferCount; ++i)
		{
			CreateUploadBuffer(true, m_bigBufferSize);
		}
		for (UINT i = 0; i < smallBufferCount; ++i)
		{
			CreateUploadBuffer(true, m_smallBufferSize);
		}
	}

	void WUploadManager::UploadDataToBuffer(VkBuffer pDestBuffer, const void* pData, size_t size, WBufferState currentState, WBufferState finalState)
	{
		WAVEE_ASSERT_MESSAGE(size <= m_bigBufferSize, "Data too big for upload buffer!");

		UINT bufferIndex = RequestUploadBuffer(size);
		UploadBuffer& uploadBuffer{ m_vUploadBuffers[bufferIndex] };

		WaveECommandBuffer pCommandBuffer = WaveManager::Instance()->GetCommandBuffer();

		memcpy(uploadBuffer.pMappedPtr, pData, size);

		if (currentState != WBufferState::TransferDst)
		{
			// Transition the buffer to the copy destination state
			VkBufferMemoryBarrier barrier{};
			barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
			barrier.srcAccessMask = GetAccessMask(currentState);
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.buffer = pDestBuffer;
			barrier.offset = 0;
			barrier.size = size;

			vkCmdPipelineBarrier(
				pCommandBuffer,
				GetPipelineStage(currentState),
				VK_PIPELINE_STAGE_TRANSFER_BIT,
				0,
				0, nullptr,
				1, &barrier,
				0, nullptr
			);
		}

		// Copy the data to the destination resource
		VkBufferCopy region{};
		region.srcOffset = 0;
		region.dstOffset = 0;
		region.size = size;
		vkCmdCopyBuffer(pCommandBuffer, uploadBuffer.pBuffer, pDestBuffer, 1, &region);

		if (finalState != WBufferState::TransferDst)
		{
			VkBufferMemoryBarrier barrier{};
			barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = GetAccessMask(finalState);
			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.buffer = pDestBuffer;
			barrier.offset = 0;
			barrier.size = size;

			vkCmdPipelineBarrier(
				pCommandBuffer,
				VK_PIPELINE_STAGE_TRANSFER_BIT,
				GetPipelineStage(finalState),
				0,
				0, nullptr,
				1, &barrier,
				0, nullptr
			);
		}
	}

	void WUploadManager::UploadDataToTexture(VkImage pDestTexture, const void* pData, UINT width, UINT height, UINT bytesPerPixel, WTextureDescriptor::Format format, WImageState currentState, WImageState finalState)
	{
		WaveECommandBuffer pCommandBuffer = WaveManager::Instance()->GetCommandBuffer();

		// Calculate required size
		UINT64 rowPitch = align_value(width * bytesPerPixel, 4);
		UINT64 totalSize = rowPitch * height;

		WAVEE_ASSERT_MESSAGE(totalSize <= m_bigBufferSize, "Texture data too big for upload buffer!");
		
		UINT bufferIndex = RequestUploadBuffer(totalSize);
		UploadBuffer& uploadBuffer{ m_vUploadBuffers[bufferIndex] };

		// Copy data to the upload buffer
		const char* pSrcData = static_cast<const char*>(pData);
		char* pDstData = static_cast<char*>(uploadBuffer.pMappedPtr);
		for (UINT y = 0; y < height; ++y)
		{
			memcpy(pDstData, pSrcData, width * bytesPerPixel);
			pSrcData += width * bytesPerPixel;
			pDstData += rowPitch;
		}

		// Transition the resource to the copy destination state if needed
		if (currentState != WImageState::TransferDst)
		{
			VkImageMemoryBarrier barrier{};
			barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			barrier.oldLayout = GetImageLayout(currentState, format);
			barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			barrier.srcAccessMask = GetImageAccessMask(currentState, format);
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.image = pDestTexture;
			barrier.subresourceRange.aspectMask = (format == WTextureDescriptor::DepthFloat) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
			barrier.subresourceRange.baseMipLevel = 0;
			barrier.subresourceRange.levelCount = 1;
			barrier.subresourceRange.baseArrayLayer = 0;
			barrier.subresourceRange.layerCount = 1;

			vkCmdPipelineBarrier(
				pCommandBuffer,
				GetImagePipelineStage(currentState, format),
				VK_PIPELINE_STAGE_TRANSFER_BIT,
				0,
				0, nullptr,
				0, nullptr,
				1, &barrier
			);
		}

		// Copy the data to the destination texture
		VkBufferImageCopy region{};
		region.bufferOffset = 0;
		region.bufferRowLength = 0;
		region.bufferImageHeight = 0;
		region.imageSubresource.aspectMask = (format == WTextureDescriptor::DepthFloat) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
		region.imageSubresource.mipLevel = 0;
		region.imageSubresource.baseArrayLayer = 0;
		region.imageSubresource.layerCount = 1;
		region.imageOffset = { 0, 0, 0 };
		region.imageExtent = { width, height, 1 };

		vkCmdCopyBufferToImage(
			pCommandBuffer,
			uploadBuffer.pBuffer,
			pDestTexture,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1,
			&region
		);

		// Transition the resource to the final state if needed
		if (finalState != WImageState::TransferDst)
		{
			VkImageMemoryBarrier barrier{};
			barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			barrier.newLayout = GetImageLayout(finalState, format);
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = GetImageAccessMask(finalState, format);
			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.image = pDestTexture;
			barrier.subresourceRange.aspectMask = (format == WTextureDescriptor::DepthFloat) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
			barrier.subresourceRange.baseMipLevel = 0;
			barrier.subresourceRange.levelCount = 1;
			barrier.subresourceRange.baseArrayLayer = 0;
			barrier.subresourceRange.layerCount = 1;

			vkCmdPipelineBarrier(
				pCommandBuffer,
				VK_PIPELINE_STAGE_TRANSFER_BIT,
				GetImagePipelineStage(finalState, format),
				0,
				0, nullptr,
				0, nullptr,
				1, &barrier
			);
		}
	}

	void WUploadManager::EndFrame()
	{
		WaveEDevice pDevice = WaveManager::Instance()->GetDevice();

		std::vector<UINT> bufferIndicesToRelease;
		for (UINT i = 0; i < m_vInUseBuffers.size(); i++)
		{
			UINT bufferIndex = m_vInUseBuffers[i];

			if (vkGetFenceStatus(pDevice, m_vUploadBuffers[bufferIndex].pFence) == VK_SUCCESS)
			{
				bufferIndicesToRelease.push_back(bufferIndex);
				vkResetFences(pDevice, 1, &m_vUploadBuffers[bufferIndex].pFence);
			}
		}
		for (UINT bufferIndex : bufferIndicesToRelease)
		{
			ReleaseUploadBuffer(bufferIndex);
		}
	}

	UINT WUploadManager::RequestUploadBuffer(size_t bufferSize)
	{
		UINT bufferIndex{ UINT_MAX };
		
		// Search for small buffers first
		if (bufferSize <= m_smallBufferSize)
		{
			for (UINT i = 0; i < m_vAvailableBuffers.size(); i++)
			{
				UINT newBufferIndex = m_vAvailableBuffers[i];
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
			for (UINT i = 0; i < m_vAvailableBuffers.size(); i++)
			{
				UINT newBufferIndex = m_vAvailableBuffers[i];
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
			auto it = std::find(m_vAvailableBuffers.begin(), m_vAvailableBuffers.end(), bufferIndex);
			WAVEE_ASSERT_MESSAGE(it != m_vAvailableBuffers.end(), "Could not find buffer index in avilable buffers!");
			m_vAvailableBuffers.erase(it);
			m_vInUseBuffers.push_back(bufferIndex);
		}

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
		m_vAvailableBuffers.push_back(bufferIndex);
	}

	UINT WUploadManager::CreateUploadBuffer(bool addToAvailableBuffers, size_t bufferSize)
	{
		WaveEDevice pDevice = WaveManager::Instance()->GetDevice();

		UploadBuffer newBuffer{};
		newBuffer.bufferSize = bufferSize;

		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = bufferSize;
		bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VkResult result = vkCreateBuffer(pDevice, &bufferInfo, nullptr, &newBuffer.pBuffer);
		WAVEE_ASSERT_MESSAGE(result == VK_SUCCESS, "Failed to create upload buffer!");

		VkMemoryRequirements memReq{};
		vkGetBufferMemoryRequirements(pDevice, newBuffer.pBuffer, &memReq);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memReq.size;
		allocInfo.memoryTypeIndex = FindMemoryType(
			memReq.memoryTypeBits,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
		);

		result = vkAllocateMemory(pDevice, &allocInfo, nullptr, &newBuffer.pMemory);
		WAVEE_ASSERT_MESSAGE(result == VK_SUCCESS, "Failed to allocate upload buffer memory!");

		vkBindBufferMemory(pDevice, newBuffer.pBuffer, newBuffer.pMemory, 0);

		vkMapMemory(pDevice, newBuffer.pMemory, 0, bufferSize, 0, &newBuffer.pMappedPtr);

		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

		vkCreateFence(pDevice, &fenceInfo, nullptr, &newBuffer.pFence);

		m_vUploadBuffers.push_back(newBuffer);
		uint32_t index = static_cast<uint32_t>(m_vUploadBuffers.size()) - 1;

		if (addToAvailableBuffers)
		{
			m_vAvailableBuffers.push_back(index);
		}
		else
		{
			m_vInUseBuffers.push_back(index);
		}

		return index;
	}
}