#pragma once

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

	VkAccessFlags GetAccessMask(WBufferState state);

	VkPipelineStageFlags GetPipelineStage(WBufferState state);

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

