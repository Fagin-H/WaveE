#include "stdafx.h"
#include "WaveManager.h"

namespace WaveE
{
	WAVEE_SINGLETON_CPP(WaveManager);

	WaveManager::WaveManager()
		: m_cbvSrvUavHeap{D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE, m_descriptorHeapCount}
		, m_rtvHeap{ D3D12_DESCRIPTOR_HEAP_TYPE_RTV, D3D12_DESCRIPTOR_HEAP_FLAG_NONE, m_descriptorHeapCount}
		, m_dsvHeap{ D3D12_DESCRIPTOR_HEAP_TYPE_DSV, D3D12_DESCRIPTOR_HEAP_FLAG_NONE, m_descriptorHeapCount}
	{
	}
}
