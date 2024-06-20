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
		};

		struct RootSignatureDescriptor
		{
			DescriptorTable* descriptorTables;
			UINT numDescriptorTables{ 0 };
		};

		WRootSigniture();

		void CreateRootSigniture(const RootSignatureDescriptor& rDescriptor);

		ID3D12RootSignature* GetRootSignature() const { return m_pRootSignature.Get(); }
	private:
		ComPtr<ID3D12RootSignature> m_pRootSignature{ nullptr };
	};
}

