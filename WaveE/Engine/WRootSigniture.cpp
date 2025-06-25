#include "stdafx.h"
#include "WRootSigniture.h"
#include "WaveManager.h"

namespace WaveE
{
	WRootSigniture::WRootSigniture()
	{
		
	}

	void WRootSigniture::CreateRootSigniture(const RootSignatureDescriptor& rDescriptor)
	{
		RootSignatureDescriptor2 rDescriptor2{};
		rDescriptor2.descriptorTables = rDescriptor.descriptorTables;
		rDescriptor2.numDescriptorTables = rDescriptor.numDescriptorTables;
		CreateRootSigniture(rDescriptor2);
	}

	void WRootSigniture::CreateRootSigniture(const RootSignatureDescriptor2& rDescriptor)
	{
		WaveEDevice* pDevice = WaveManager::Instance()->GetDevice();

		struct IndicesForDescriptorRanges
		{
			UINT indexToStartOfDescriptorRanges;
			UINT indexToEndOfDescriptorRanges;
		};
		std::vector<D3D12_ROOT_PARAMETER> vRootParameters;
		std::vector<IndicesForDescriptorRanges> vIndicesForDescriptorRanges;
		std::vector<D3D12_DESCRIPTOR_RANGE> vDescriptorRanges;

		UINT CBVCount{ 0 };
		UINT SRVCount{ 0 };
		UINT UAVCount{ 0 };
		UINT SamplerCount{ 0 };

		// ----- Descriptor Tables -----
		for (UINT i = 0; i < rDescriptor.numDescriptorTables; i++)
		{
			DescriptorTable& rDT = rDescriptor.descriptorTables[i];
			bool isSamplerDT{ true };
			if (rDT.numCBVs + rDT.numSRVs + rDT.numUAVs > 0)
			{
				WAVEE_ASSERT_MESSAGE(rDT.numSamplers == 0, "Descriptor table can't have samplers and non samplers!");
				isSamplerDT = false;
			}

			D3D12_ROOT_PARAMETER newRootParamater;
			IndicesForDescriptorRanges newIndicesForDescriptorRanges;
			newIndicesForDescriptorRanges.indexToStartOfDescriptorRanges = vDescriptorRanges.size();
			// For now shader visibility for all. This should be updated in the future.
			newRootParamater.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
			newRootParamater.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;

			if (isSamplerDT)
			{
				D3D12_DESCRIPTOR_RANGE range = {};
				range.BaseShaderRegister = SamplerCount;
				range.NumDescriptors = rDT.numSamplers;
				range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
				range.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
				range.RegisterSpace = rDT.space;

				SamplerCount += rDT.numSamplers;
				vDescriptorRanges.push_back(range);
			}
			else
			{
				if (rDT.numCBVs > 0)
				{
					D3D12_DESCRIPTOR_RANGE range = {};
					range.BaseShaderRegister = CBVCount;
					range.NumDescriptors = rDT.numCBVs;
					range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
					range.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
					range.RegisterSpace = rDT.space;

					CBVCount += rDT.numCBVs;
					vDescriptorRanges.push_back(range);
				}
				if (rDT.numSRVs > 0)
				{
					D3D12_DESCRIPTOR_RANGE range = {};
					range.BaseShaderRegister = SRVCount;
					range.NumDescriptors = rDT.numSRVs;
					range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
					range.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
					range.RegisterSpace = rDT.space;

					SRVCount += rDT.numSRVs;
					vDescriptorRanges.push_back(range);
				}
				if (rDT.numUAVs > 0)
				{
					D3D12_DESCRIPTOR_RANGE range = {};
					range.BaseShaderRegister = UAVCount;
					range.NumDescriptors = rDT.numUAVs;
					range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
					range.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
					range.RegisterSpace = rDT.space;

					UAVCount += rDT.numUAVs;
					vDescriptorRanges.push_back(range);
				}
			}

			newIndicesForDescriptorRanges.indexToEndOfDescriptorRanges = vDescriptorRanges.size();

			vRootParameters.push_back(newRootParamater);
			vIndicesForDescriptorRanges.push_back(newIndicesForDescriptorRanges);
		}

		for (UINT i = 0; i < vRootParameters.size(); i++)
		{
			// Update root parameters here so pointers to descriptor ranges don't become invalid
			UINT startIndex = vIndicesForDescriptorRanges[i].indexToStartOfDescriptorRanges;
			UINT endIndex = vIndicesForDescriptorRanges[i].indexToEndOfDescriptorRanges;
			UINT count = endIndex - startIndex;

			vRootParameters[i].DescriptorTable.NumDescriptorRanges = count;
			vRootParameters[i].DescriptorTable.pDescriptorRanges = &vDescriptorRanges[startIndex];
		}

		// ----- Root Descriptors -----
		for (UINT i = 0; i < rDescriptor.numRootDescriptors; ++i)
		{
			D3D12_ROOT_PARAMETER param = {};
			param.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

			const RootDescriptor& rRD = rDescriptor.rootDescriptors[i];

			switch (rRD.type)
			{
			case RootDescriptor::CBV:
				param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
				param.Descriptor.ShaderRegister = CBVCount;
				param.Descriptor.RegisterSpace = rRD.space;
				CBVCount++;
				break;

			case RootDescriptor::SRV:
				param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
				param.Descriptor.ShaderRegister = SRVCount;
				param.Descriptor.RegisterSpace = rRD.space;
				SRVCount++;
				break;

			case RootDescriptor::UAV:
				param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_UAV;
				param.Descriptor.ShaderRegister = UAVCount;
				param.Descriptor.RegisterSpace = rRD.space;
				UAVCount++;
				break;
			}

			vRootParameters.push_back(param);
		}

		// ----- Root Constants -----
		for (UINT i = 0; i < rDescriptor.numRootConstants; i++)
		{
			const RootConstant& rRC = rDescriptor.rootConstants[i];

			D3D12_ROOT_PARAMETER rootParam{};
			rootParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
			rootParam.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
			rootParam.Constants.ShaderRegister = CBVCount;
			rootParam.Constants.RegisterSpace = rRC.space;
			rootParam.Constants.Num32BitValues = rRC.num32BitValues;
			CBVCount++;

			vRootParameters.push_back(rootParam);
		}

		D3D12_ROOT_SIGNATURE_DESC rootSignitureDescriptor;
		rootSignitureDescriptor.NumParameters = vRootParameters.size();
		rootSignitureDescriptor.pParameters = vRootParameters.data();
		rootSignitureDescriptor.NumStaticSamplers = 0;
		rootSignitureDescriptor.pStaticSamplers = nullptr;
		rootSignitureDescriptor.Flags = rDescriptor.bIsLocalRootSignature ? D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE : D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;


		ComPtr<ID3DBlob> signature;
		ComPtr<ID3DBlob> error;
		HRESULT hr = D3D12SerializeRootSignature(&rootSignitureDescriptor, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error);
		WAVEE_ASSERT_MESSAGE(SUCCEEDED(hr), "Failed to serialize root signature!");

		hr = pDevice->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_pRootSignature));
		WAVEE_ASSERT_MESSAGE(SUCCEEDED(hr), "Failed to create root signature!");
	}

}