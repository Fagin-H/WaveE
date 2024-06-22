#pragma once
#include "WDescriptorHeapManager.h"

namespace WaveE
{
	struct WSamplerDescriptor
	{
		enum Filter
		{
			Point,
			Linear,
			Anisotropic
		};

		enum AddressMode
		{
			Wrap,
			Clamp,
			Border
		};

		Filter filter{ Linear };
		AddressMode addressMode{ Wrap };
		float mipLODBias{ 0.0f };
		UINT maxAnisotropy{ 1 };
		float borderColor[4]{ 1.0f, 1.0f, 1.0f, 1.0f };
		float minLOD{ 0.0f };
		float maxLOD{ D3D12_FLOAT32_MAX };
	};

	class WSampler
	{
	public:
		WSampler(const WSamplerDescriptor& rDescriptor, WDescriptorHeapManager::Allocation allocation = WDescriptorHeapManager::InvalidAllocation(), UINT offset = 0);
		~WSampler();

		D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle() const;
		WDescriptorHeapManager::Allocation GetAllocation() const { return m_allocation; }
	private:
		WDescriptorHeapManager::Allocation m_allocation;
		UINT m_offset;
		bool m_doesOwnAllocation;
	};
}
