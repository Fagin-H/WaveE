#pragma once

namespace WaveE
{
	class WRootSigniture
	{
	public:
		struct DescriptorTable
		{
			UINT numCBVs{ 0 };
			UINT numSRVs{ 0 };
			UINT numUAVs{ 0 };
			UINT numSamplers{ 0 };
			UINT space{ 0 };
		};

		struct RootDescriptor
		{
			enum Type
			{
				CBV,
				SRV,
				UAV
			} type;
			UINT space{ 0 };
		};

		struct RootConstant
		{
			UINT num32BitValues{ 0 };
			UINT space{ 0 };
		};

		struct RootSignatureDescriptor
		{
			DescriptorTable* descriptorTables;
			UINT numDescriptorTables{ 0 };
		};

		struct RootSignatureDescriptor2
		{
			DescriptorTable* descriptorTables{ nullptr };
			RootDescriptor* rootDescriptors{ nullptr };
			RootConstant* rootConstants{ nullptr };
			UINT numDescriptorTables{ 0 };
			UINT numRootDescriptors{ 0 };
			UINT numRootConstants{ 0 };
			bool bIsLocalRootSignature{ false };
		};

		WRootSigniture();

		void CreateRootSigniture(const RootSignatureDescriptor& rDescriptor);
		void CreateRootSigniture(const RootSignatureDescriptor2& rDescriptor);

		ID3D12RootSignature* GetRootSignature() const { return m_pRootSignature.Get(); }
		ID3D12RootSignature* const * GetRootSignatureAddress() const { return m_pRootSignature.GetAddressOf(); }
	private:
		ComPtr<ID3D12RootSignature> m_pRootSignature{ nullptr };
	};
}

