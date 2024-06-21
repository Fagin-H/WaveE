#pragma once
#include "WDescriptorHeapManager.h"
#include <functional>
#include "WUploadManager.h"
#include "WRootSigniture.h"
#include "WSampler.h"
#include "WResourceManager.h"


namespace WaveE
{
	// Manager class for WaveE
	class WaveManager
	{
		WAVEE_SINGLETON(WaveManager)

	public:
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
		WDescriptorHeapManager* GetSamplerHeap() { return &m_sampelerHeap; }

		WUploadManager* GetUploadManager() { return &m_uploadManager; }

		WRootSigniture* GetDefaultRootSigniture() { return &m_defaultRootSigniture; }

		enum SamplerType
		{
			WRAP_POINT,
			WRAP_LINEAR,
			CLAMP_POINT,
			CLAMP_LINEAR
		};
		ResourceID<WSampler> GetDefaultSampler(SamplerType type = WRAP_LINEAR);

	private:
		WaveManager();
		~WaveManager();
		void CreateDefaultRootSigniture();
		// Create the hlsl include file for named slots
		// If the default slot layout changes the shaders don't need to be changes
		// Just replace the old hlsli file with the new one and recompile
		void CreateSlotHLSLIFile();

		// DX12 variables
		ComPtr<WaveEDevice> m_pDevice;
		// #TODO have multiple command lists. Would require careful consideration of synchronization
		ComPtr<WaveECommandList> m_pCommandList;
		ComPtr<WaveECommandQueue> m_CommandQueue;

		// WaveE variables
		WDescriptorHeapManager m_cbvSrvUavHeap;
		WDescriptorHeapManager m_rtvHeap;
		WDescriptorHeapManager m_dsvHeap;
		WDescriptorHeapManager m_sampelerHeap;

		WUploadManager m_uploadManager;

		WRootSigniture m_defaultRootSigniture;

		ResourceBlock<WSampler> m_defaultSamplers;

		const UINT m_descriptorHeapCountCBV_SRV_UAV{ 1024 };
		const UINT m_descriptorHeapCountRTV{ 16 };
		const UINT m_descriptorHeapCountDSV{ 16 };
		const UINT m_descriptorHeapCountSampler{ 16 };

		const UINT m_uploadBufferCount{ 10 };
		const UINT m_uploadBufferSize{ 1024 * 1024 * 32 };

		const UINT m_defaultSamplerCount{ 4 };
		const UINT m_globalCBVCount{ 1 };
		const UINT m_globalSRVCount{ 4 };
		const UINT m_globalSamplerCount{ m_globalSRVCount };
		const UINT m_materialCBVCount{ 1 };
		const UINT m_materialSRVCount{ 4 };
		const UINT m_materialSamplerCount{ m_materialSRVCount };
	};
}

