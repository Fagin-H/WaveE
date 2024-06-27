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

		void UploadDataToBuffer(ID3D12Resource* pDestResource, const void* pData, size_t size, D3D12_RESOURCE_STATES currentState, D3D12_RESOURCE_STATES finalState);
		void UploadDataToTexture(ID3D12Resource* pDestResource, const void* pData, UINT bytesPerPixel, D3D12_RESOURCE_STATES currentState, D3D12_RESOURCE_STATES finalState);

	private:
		struct UploadBuffer
		{
			ComPtr<ID3D12Resource> pResource{ nullptr };
			UINT64 fenceValue{ 0 };
			size_t bufferSize;
		};

		size_t m_bigBufferSize;
		size_t m_smallBufferSize;
		std::vector<UploadBuffer> m_vUploadBuffers;
		std::vector<UINT> m_qAvailableBuffers;
		std::vector<UINT> m_vInUseBuffers;

		HANDLE m_fenceEvent;
		ComPtr<ID3D12Fence> m_pFence;
		UINT64 m_fenceValue;

		UINT RequestUploadBuffer(size_t bufferSize);
		void ReleaseUploadBuffer(UINT bufferIndex);
		UINT CreateUploadBuffer(bool addToAvailableBuffers, size_t bufferSize);
		
		friend class WaveManager;
		void EndFrame();
	};
}

