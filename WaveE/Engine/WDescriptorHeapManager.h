#pragma once

namespace WaveE
{
	// A class for managing descriptor heaps
	class WDescriptorHeapManager
	{
	public:
		WAVEE_NO_COPY(WDescriptorHeapManager)
		
		struct Allocation
		{
			UINT index{ UINT_MAX };
			UINT size{ 0 };

			bool IsValid() const
			{
				return index != UINT_MAX;
			}
		};

		WDescriptorHeapManager() = default;
		~WDescriptorHeapManager();

		void Init(D3D12_DESCRIPTOR_HEAP_TYPE type, D3D12_DESCRIPTOR_HEAP_FLAGS flags, UINT numDescriptors);

		Allocation Allocate(UINT size = 1);
		void Deallocate(Allocation allocation);

		CD3DX12_GPU_DESCRIPTOR_HANDLE GetGPUHandle(UINT index) const;
		CD3DX12_GPU_DESCRIPTOR_HANDLE GetGPUHandle(Allocation allocation) const { return GetGPUHandle(allocation.index); }
		CD3DX12_CPU_DESCRIPTOR_HANDLE GetCPUHandle(UINT index) const;
		CD3DX12_CPU_DESCRIPTOR_HANDLE GetCPUHandle(Allocation allocation) const { return GetCPUHandle(allocation.index); }

		static Allocation InvalidAllocation() { return Allocation{ UINT_MAX, 0 }; }
		static bool IsInvalidAllocation(Allocation allocation) { return allocation.index == UINT_MAX; }

		ID3D12DescriptorHeap* GetHeap() { return m_pDescriptorHeap.Get(); }

	private:
		ComPtr<ID3D12DescriptorHeap> m_pDescriptorHeap;
		UINT m_descriptorSize;
		UINT m_numDescriptors;

		struct HeapIndex
		{
			UINT index;
			bool isFree;
		};
		std::vector<HeapIndex> m_heapIndices;
	};
}

