#pragma once
#include "WDescriptorHeapManager.h"

namespace WaveE
{
	struct WBufferDescriptor
	{
		enum Type
		{
			Constant,
			Vertex,
			Index,
			SRV,
			UAV
		};

		bool isDynamic{ false };
		size_t m_sizeBytes{ 0 };
		Type type{ Constant };
		const void* pInitalData{ nullptr };
	};

	// A wrapper around a DX12 resource for use as a buffer
	class WBuffer
	{
	public:
		WBuffer(const WBufferDescriptor& rDescriptor, WDescriptorHeapManager::Allocation allocation = WDescriptorHeapManager::InvalidAllocation(), UINT offset = 0);
		~WBuffer();

		ID3D12Resource* GetBuffer() { return m_pBuffer.Get(); }

		void UploadData(const void* pData, size_t sizeBytes);

		D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle() const;
	private:
		size_t m_sizeBytes{ 0 };
		WBufferDescriptor::Type m_type;
		D3D12_RESOURCE_STATES m_state;
		WDescriptorHeapManager::Allocation m_allocation;
		UINT m_offset;
		bool m_doesOwnAllocation;
		ComPtr<ID3D12Resource> m_pBuffer{ nullptr };
	};
}

