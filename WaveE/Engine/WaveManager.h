#pragma once
#include "WDescriptorHeapManager.h"
#include <functional>
#include "WUploadManager.h"

namespace WaveE
{
	// Manager class for WaveE
	class WaveManager
	{
		WAVEE_SINGLETON(WaveManager)

	public:
		WaveManager();

		void BeginFrame();
		void Update();
		void Render();
		void EndFrame();

		// Getter functions
		WaveEDevice* GetDevice() const { return m_pDevice.Get(); }
		WaveECommandList* GetCommandList() const { return m_pCommandList.Get(); }
		WaveECommandQueue* GetCommandQueue() const { return m_CommandQueue.Get(); }

		WDescriptorHeapManager* GetCBV_SRV_UAVHeap() { return &m_cbvSrvUavHeap; }
		WDescriptorHeapManager* GetRTVHeap() { return &m_rtvHeap; }
		WDescriptorHeapManager* GetDSVHeap() { return &m_dsvHeap; }

		WUploadManager* GetUploadManager() { return &m_uploadManager; }

	private:
		// DX12 variables
		ComPtr<WaveEDevice> m_pDevice;
		// #TODO have multiple command lists. Would require careful consideration of synchronization
		ComPtr<WaveECommandList> m_pCommandList;
		ComPtr<WaveECommandQueue> m_CommandQueue;

		// WaveE variables
		WDescriptorHeapManager m_cbvSrvUavHeap;
		WDescriptorHeapManager m_rtvHeap;
		WDescriptorHeapManager m_dsvHeap;

		WUploadManager m_uploadManager;

		const UINT m_descriptorHeapCount{ 1024 };
		const UINT m_uploadBufferCount{ 10 };
		const UINT m_uploadBufferSize{ 1024 * 1024 * 32 };
	};
}

