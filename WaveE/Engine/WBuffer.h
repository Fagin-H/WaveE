#pragma once
#include "WDescriptorHeapManager.h"

namespace WaveE
{
	struct WBufferDescriptor
	{
		enum Type
		{
			Uniform,
			Vertex,
			Index,
			StorageRO,
			StorageRW
		};

		bool isDynamic{ false };
		size_t sizeBytes{ 0 };
		Type type{ Uniform };
		int descriptorSlot{ -1 };
		const void* pInitalData{ nullptr };
	};

	enum class WBufferState
	{
		Undefined,
		TransferDst,
		TransferSrc,
		VertexBuffer,
		IndexBuffer,
		UniformBuffer,
		StorageBufferRead,
		StorageBufferWrite
	};

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

	class WBuffer
	{
	public:
		WBuffer(const WBufferDescriptor& rDescriptor);
		~WBuffer();

		VkBuffer GetBuffer() const { return m_pBuffer; }

		void UploadData(const void* pData, size_t sizeBytes);

		int GetSlot() const { return m_slot; }

		bool IsStorage() const { return m_type == WBufferDescriptor::StorageRO || m_type == WBufferDescriptor::StorageRW; }

		void Transition(WBufferState newState);
	private:
		size_t m_sizeBytes{ 0 };
		WBufferDescriptor::Type m_type;
		WBufferState m_bufferState{ WBufferState::Undefined };
		int m_slot{ -1 };
		bool m_isDynamic{ false };
		VkDeviceMemory m_pMemory{ nullptr };
		VkBuffer m_pBuffer{ nullptr };
	};
}

