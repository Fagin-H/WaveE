#pragma once
#include "WDescriptorHeapManager.h"

namespace WaveE
{
	// Manager class for WaveE
	class WaveManager
	{
		WAVEE_SINGLETON(WaveManager)

	public:
		WaveManager();

		// Getter functions
		ID3D12Device1* GetDevice() const { return m_pDevice.Get(); }

		WDescriptorHeapManager* GetCBV_SRV_UAVHeap() { return &m_cbvSrvUavHeap; }
		WDescriptorHeapManager* GetRTVHeap() { return &m_rtvHeap; }
		WDescriptorHeapManager* GetDSVHeap() { return &m_dsvHeap; }

	private:
		// DX12 variables
		ComPtr<ID3D12Device1> m_pDevice;

		// WaveE variables
		WDescriptorHeapManager m_cbvSrvUavHeap;
		WDescriptorHeapManager m_rtvHeap;
		WDescriptorHeapManager m_dsvHeap;

		const UINT m_descriptorHeapCount{ 1024 };
	};
}

