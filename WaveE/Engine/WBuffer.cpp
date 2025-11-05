#include "stdafx.h"
#include "WBuffer.h"
#include "WaveManager.h"

namespace WaveE
{
	VkBufferUsageFlags GetUseageFlags(const WBufferDescriptor& rDescriptor)
	{
		VkBufferUsageFlags usageFlags = VK_BUFFER_USAGE_TRANSFER_DST_BIT;

		switch (rDescriptor.type) {
			case WBufferDescriptor::Uniform: usageFlags |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT; break;
			case WBufferDescriptor::StorageRO: [[fallthrough]];
			case WBufferDescriptor::StorageRW: usageFlags |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT; break;
			case WBufferDescriptor::Vertex: usageFlags |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT; break;
			case WBufferDescriptor::Index: usageFlags |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT; break;
		}

		return usageFlags;
	}

	WBuffer::WBuffer(const WBufferDescriptor& rDescriptor)
	{
		bool initialData = rDescriptor.pInitalData;
		m_sizeBytes = rDescriptor.sizeBytes;
		m_type = rDescriptor.type;
		m_isDynamic = rDescriptor.isDynamic;

		WaveEDevice pDevice = WaveManager::Instance()->GetDevice();

		WAVEE_ASSERT_MESSAGE(pDevice, "Failed to get device!");

		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = rDescriptor.sizeBytes;
		bufferInfo.usage = GetUseageFlags(rDescriptor);
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VkResult result = vkCreateBuffer(pDevice, &bufferInfo, nullptr, &m_pBuffer);
		WAVEE_ASSERT_MESSAGE(result == VK_SUCCESS, "Failed to create buffer!");

		VkMemoryRequirements memReq;
		vkGetBufferMemoryRequirements(pDevice, m_pBuffer, &memReq);

		uint32_t memoryTypeIndex = FindMemoryType(
			memReq.memoryTypeBits,
			rDescriptor.isDynamic
			? (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
			: VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
		);

		VkMemoryAllocateInfo allocateInfo{};
		allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocateInfo.allocationSize = memReq.size;
		allocateInfo.memoryTypeIndex = memoryTypeIndex;

		result = vkAllocateMemory(pDevice, &allocateInfo, nullptr, &m_pMemory);
		WAVEE_ASSERT_MESSAGE(result == VK_SUCCESS, "Failed to allocate memory!");

		result = vkBindBufferMemory(pDevice, m_pBuffer, m_pMemory, 0);
		WAVEE_ASSERT_MESSAGE(result == VK_SUCCESS, "Failed to bind buffer memory!");

		if (rDescriptor.type != WBufferDescriptor::Vertex && rDescriptor.type != WBufferDescriptor::Index)
		{
			m_slot = WaveManager::Instance()->GetDescriptorManager()->AddResource(m_pBuffer, IsStorage(), rDescriptor.descriptorSlot);
		}

		if (initialData)
		{
			UploadData(rDescriptor.pInitalData, m_sizeBytes);
		}
	}

	WBuffer::~WBuffer()
	{
	}

	void WBuffer::UploadData(const void* pData, size_t sizeBytes)
	{
		WAVEE_ASSERT_MESSAGE(sizeBytes <= m_sizeBytes, "Data too big for buffer!");

		if (m_isDynamic) 
		{
			WaveEDevice pDevice = WaveManager::Instance()->GetDevice();
			WAVEE_ASSERT_MESSAGE(pDevice, "Failed to get device!");

			void* mapped;
			vkMapMemory(pDevice, m_pMemory, 0, sizeBytes, 0, &mapped);
			memcpy(mapped, pData, sizeBytes);
			vkUnmapMemory(pDevice, m_pMemory);
		}
		else 
		{
			WaveManager::Instance()->GetUploadManager()->UploadDataToBuffer(m_pBuffer, pData, sizeBytes, m_bufferState, m_bufferState);
		}
	}

	void WBuffer::Transition(WBufferState newState)
	{
		if (m_bufferState == newState)
		{
			return;
		}

		WaveECommandBuffer pCmdBuffer = WaveManager::Instance()->GetCommandBuffer();

		VkBufferMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
		barrier.buffer = m_pBuffer;
		barrier.offset = 0;
		barrier.size = VK_WHOLE_SIZE;

		barrier.srcAccessMask = GetAccessMask(m_bufferState);
		barrier.dstAccessMask = GetAccessMask(newState);
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

		vkCmdPipelineBarrier(
			pCmdBuffer,
			GetPipelineStage(m_bufferState),
			GetPipelineStage(newState),
			0,
			0, nullptr,
			1, &barrier,
			0, nullptr
		);

		m_bufferState = newState;
	}

	VkAccessFlags GetAccessMask(WBufferState state)
	{
		switch (state)
		{
			case WBufferState::Undefined:
				return 0;
			case WBufferState::TransferDst:
				return VK_ACCESS_TRANSFER_WRITE_BIT;
			case WBufferState::TransferSrc:
				return VK_ACCESS_TRANSFER_READ_BIT;
			case WBufferState::VertexBuffer:
				return VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
			case WBufferState::IndexBuffer:
				return VK_ACCESS_INDEX_READ_BIT;
			case WBufferState::UniformBuffer:
				return VK_ACCESS_UNIFORM_READ_BIT;
			case WBufferState::StorageBufferRead:
				return VK_ACCESS_SHADER_READ_BIT;
			case WBufferState::StorageBufferWrite:
				return VK_ACCESS_SHADER_WRITE_BIT;
			default:
				return 0;
		}
	}
	VkPipelineStageFlags GetPipelineStage(WBufferState state)
	{
		switch (state)
		{
			case WBufferState::Undefined:
				return VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			case WBufferState::TransferDst: [[fallthrough]]
			case WBufferState::TransferSrc:
				return VK_PIPELINE_STAGE_TRANSFER_BIT;
			case WBufferState::VertexBuffer: [[fallthrough]]
			case WBufferState::IndexBuffer:
				return VK_PIPELINE_STAGE_VERTEX_INPUT_BIT;
			case WBufferState::UniformBuffer:
				return VK_PIPELINE_STAGE_VERTEX_SHADER_BIT |
					VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			case WBufferState::StorageBufferRead:
				return VK_PIPELINE_STAGE_VERTEX_SHADER_BIT |
					VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT |
					VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
			case WBufferState::StorageBufferWrite:
				return VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT |
					VK_PIPELINE_STAGE_VERTEX_SHADER_BIT |
					VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			default:
				return VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		}
	}
}
