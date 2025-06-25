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
			UAV,
			RAY_TRACING_AS,
			RAY_TRACING_VERTEX,
			DESCRIPTOR,
		};

		bool isDynamic{ false };
		bool isUpload{ false };
		size_t sizeBytes{ 0 };
		UINT height{ 0 };
		UINT strideBytes{ 0 };
		Type type{ Constant };
		const void* pInitalData{ nullptr };
		// UAV
		D3D12_UAV_DIMENSION UAVDimention{ D3D12_UAV_DIMENSION_TEXTURE2D };
		DXGI_FORMAT UAVFormat{ DXGI_FORMAT_R8G8B8A8_UNORM };
	};

	// A wrapper around a DX12 resource for use as a buffer
	class WBuffer
	{
	public:
		WBuffer(const WBufferDescriptor& rDescriptor, WDescriptorHeapManager::Allocation allocation = WDescriptorHeapManager::InvalidAllocation(), UINT offset = 0);
		~WBuffer();

		ID3D12Resource* GetBuffer() { return m_pBuffer.Get(); }

		void UploadData(const void* pData, size_t sizeBytes, UINT offsetBytes = 0);

		D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle() const;

		WDescriptorHeapManager::Allocation GetAllocation() const { return m_allocation; }
		size_t GetSize() const { return m_sizeBytes; }
		D3D12_RESOURCE_STATES GetCurrentState() const { return m_state; }

		void CreateView(WDescriptorHeapManager::Allocation allocationSRV, UINT offset = 0);
	private:
		size_t m_sizeBytes{ 0 };
		UINT m_strideBytes{ 0 };
		UINT m_height{ 0 };
		WBufferDescriptor::Type m_type;
		D3D12_RESOURCE_STATES m_state;
		WDescriptorHeapManager::Allocation m_allocation;
		UINT m_offset;
		bool m_bDoesOwnAllocation;
		bool m_bIsUploadBuffer;
		bool m_bHasView{ true };
		DXGI_FORMAT m_UAVFormat;
		ComPtr<ID3D12Resource> m_pBuffer{ nullptr };
	};
}

