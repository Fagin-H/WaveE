#pragma once
namespace WaveE
{
	struct WBufferDescriptor
	{
		enum Type
		{
			Constant,
			Vertex,
			Dynamic,
			Index,
			SRV,
			UAV
		};

		size_t m_sizeBytes{ 0 };
		Type type{ Constant };
		const void* m_pInitalData{ nullptr };
	};

	// A wrapper around a DX12 resource for use as a buffer
	class WBuffer
	{
		WBuffer(const WBufferDescriptor& rDescriptor);

		ID3D12Resource* GetBuffer() { return m_pBuffer.Get(); }

		void UploadData(const void* pData, size_t sizeBytes);
	private:
		size_t m_sizeBytes{ 0 };
		WBufferDescriptor::Type m_type;
		ComPtr<ID3D12Resource> m_pBuffer{ nullptr };
	};
}

