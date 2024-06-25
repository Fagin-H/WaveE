#include "stdafx.h"
#include "WSampler.h"
#include "WaveManager.h"

namespace WaveE
{
	D3D12_FILTER ConvertFilter(const WSamplerDescriptor::Filter filter)
	{
		switch (filter)
		{
		case WSamplerDescriptor::Point: return D3D12_FILTER_MIN_MAG_MIP_POINT;
		case WSamplerDescriptor::Linear: return D3D12_FILTER_MIN_MAG_MIP_LINEAR;
		case WSamplerDescriptor::Anisotropic: return D3D12_FILTER_ANISOTROPIC;
		}
		return D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	}

	D3D12_TEXTURE_ADDRESS_MODE ConvertAddressMode(const WSamplerDescriptor::AddressMode addressMode)
	{
		switch (addressMode)
		{
		case WSamplerDescriptor::Wrap: return D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		case WSamplerDescriptor::Clamp: return D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		case WSamplerDescriptor::Border: return D3D12_TEXTURE_ADDRESS_MODE_BORDER;
		}
		return D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	}

	WSampler::WSampler(const WSamplerDescriptor& rDescriptor, WDescriptorHeapManager::Allocation allocation, UINT offset)
		: m_allocation{allocation}
		, m_offset{offset}
		, m_doesOwnAllocation{ WDescriptorHeapManager::IsInvalidAllocation(allocation) }
	{

		D3D12_SAMPLER_DESC samplerDesc = {};
		samplerDesc.Filter = ConvertFilter(rDescriptor.filter);
		samplerDesc.AddressU = ConvertAddressMode(rDescriptor.addressMode);
		samplerDesc.AddressV = ConvertAddressMode(rDescriptor.addressMode);
		samplerDesc.AddressW = ConvertAddressMode(rDescriptor.addressMode);
		samplerDesc.MipLODBias = rDescriptor.mipLODBias;
		samplerDesc.MaxAnisotropy = rDescriptor.maxAnisotropy;
		samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
		memcpy(samplerDesc.BorderColor, rDescriptor.borderColor, sizeof(samplerDesc.BorderColor));
		samplerDesc.MinLOD = rDescriptor.minLOD;
		samplerDesc.MaxLOD = rDescriptor.maxLOD;

		WaveEDevice* pDevice = WaveManager::Instance()->GetDevice();
		
		WDescriptorHeapManager* pSamplerHeapManager = WaveManager::Instance()->GetSamplerHeap();
		if (m_doesOwnAllocation)
		{
			m_allocation = pSamplerHeapManager->Allocate();
		}
		D3D12_CPU_DESCRIPTOR_HANDLE cpuDescriptorHandle = pSamplerHeapManager->GetCPUHandle(m_allocation.index + m_offset);
		pDevice->CreateSampler(&samplerDesc, cpuDescriptorHandle);
	}

	WSampler::~WSampler()
	{
		if (m_doesOwnAllocation)
		{
			if (!WDescriptorHeapManager::IsInvalidAllocation(m_allocation))
			{
				WDescriptorHeapManager* pSamplerHeapManager = WaveManager::Instance()->GetSamplerHeap();
				pSamplerHeapManager->Deallocate(m_allocation);
			}
		}
	}

	D3D12_CPU_DESCRIPTOR_HANDLE WSampler::GetCPUDescriptorHandle() const
	{
		WDescriptorHeapManager* pSamplerHeapManager = WaveManager::Instance()->GetSamplerHeap();
		return pSamplerHeapManager->GetCPUHandle(m_allocation.index + m_offset);
	}
}
