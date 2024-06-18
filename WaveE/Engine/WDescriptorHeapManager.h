#pragma once
#include <stack>

namespace WaveE
{
	// A class for managing descriptor heaps
	class WDescriptorHeapManager
	{
	public:
		WDescriptorHeapManager(D3D12_DESCRIPTOR_HEAP_TYPE type, D3D12_DESCRIPTOR_HEAP_FLAGS flags, UINT numDescriptors);

		UINT Allocate();
		void Deallocate(UINT index);

		CD3DX12_GPU_DESCRIPTOR_HANDLE GetGPUHandle(UINT index) const;
		CD3DX12_CPU_DESCRIPTOR_HANDLE GetCPUHandle(UINT index) const;

	private:
		ComPtr<ID3D12DescriptorHeap> m_pDescriptorHeap;
		UINT m_descriptorSize;
		UINT m_numDescriptors;
		std::stack<UINT> m_freeList;
	};
}

