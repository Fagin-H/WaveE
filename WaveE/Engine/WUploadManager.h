#pragma once

namespace WaveE
{
	// A class to manage upload buffers for moving data from the CPU to the GPU
	// Uses multiple upload buffers and fences to reuse buffers when they are done
	// #TODO Allow for multiple sized buffers so it can efficiently upload large and small amounts of data
	class WUploadManager
	{
	public:
		WAVEE_NO_COPY(WUploadManager)
		WUploadManager() = default;
		~WUploadManager();

		void Init(size_t bigBufferSize, UINT bigBufferCount, size_t smallBufferSize, UINT smallBufferCount);

		void UploadDataToBuffer(VkBuffer pDestBuffer, const void* pData, size_t size, WBufferState currentState, WBufferState finalState);
		void UploadDataToTexture(VkImage pDestTexture, const void* pData, UINT width, UINT height, UINT bytesPerPixel, WTextureDescriptor::Format format, WImageState currentState, WImageState finalState);

	private:
		struct UploadBuffer
		{
			VkBuffer pBuffer{ VK_NULL_HANDLE };
			VkDeviceMemory pMemory{ VK_NULL_HANDLE };
			VkFence pFence{ VK_NULL_HANDLE };
			size_t bufferSize{ 0 };
			void* pMappedPtr{ nullptr };
		};

		size_t m_bigBufferSize;
		size_t m_smallBufferSize;
		std::vector<UploadBuffer> m_vUploadBuffers;
		std::vector<UINT> m_vAvailableBuffers;
		std::vector<UINT> m_vInUseBuffers;

		UINT RequestUploadBuffer(size_t bufferSize);
		void ReleaseUploadBuffer(UINT bufferIndex);
		UINT CreateUploadBuffer(bool addToAvailableBuffers, size_t bufferSize);
		
		friend class WaveManager;
		void EndFrame();
	};
}

