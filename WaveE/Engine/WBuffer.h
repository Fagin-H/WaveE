#pragma once
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
		const void* m_pInitalData{ nullptr };
	};

	// A wrapper around a DX12 resource for use as a buffer
	class WBuffer
	{
	public:
		WBuffer(const WBufferDescriptor& rDescriptor);
		~WBuffer();

		ID3D12Resource* GetBuffer() { return m_pBuffer.Get(); }

		void UploadData(const void* pData, size_t sizeBytes);

		D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle() const;
	private:
		size_t m_sizeBytes{ 0 };
		WBufferDescriptor::Type m_type;
		UINT m_cpuDescriptorHandleIndex;
		ComPtr<ID3D12Resource> m_pBuffer{ nullptr };
	};
}

