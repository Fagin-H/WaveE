#pragma once
#include "WTexture.h"
#include "WBuffer.h"
#include "WSampler.h"

namespace WaveE
{
	template<typename Resource>
	struct ResourceID
	{
		UINT id;

		ResourceID* GetResource() const
		{
			return WResourceManager::Instance()->GetResource<Resource>(*this);
		}
	};

	template<typename Resource>
	struct ResourceBlock
	{
		UINT startID;
		UINT numElements;
		WDescriptorHeapManager::Allocation allocation;

		ResourceID<Resource> GetResorce(UINT index)
		{
			WAVEE_ASSERT_MESSAGE(index < numElements, "Resource index out of range!");
			return ResourceID<Resource>{startID + index};
		}
	};

	class WResourceManager
	{
		WAVEE_SINGLETON(WResourceManager)

	public:
		ResourceID<WTexture> CreateResource(const WTextureDescriptor& rDescriptor, WDescriptorHeapManager::Allocation allocation = WDescriptorHeapManager::InvalidAllocation(), UINT offset = 0);
		ResourceID<WBuffer> CreateResource(const WBufferDescriptor& rDescriptor, WDescriptorHeapManager::Allocation allocation = WDescriptorHeapManager::InvalidAllocation(), UINT offset = 0);
		ResourceID<WSampler> CreateResource(const WSamplerDescriptor& rDescriptor, WDescriptorHeapManager::Allocation allocation = WDescriptorHeapManager::InvalidAllocation(), UINT offset = 0);

		ResourceBlock<WTexture> CreateResourceBlock(WTextureDescriptor* pDescriptors, UINT numDescriptors);
		ResourceBlock<WBuffer> CreateResourceBlock(WBufferDescriptor* pDescriptors, UINT numDescriptors);
		ResourceBlock<WSampler> CreateResourceBlock(WSamplerDescriptor* pDescriptors, UINT numDescriptors);

		WTexture* GetResource(ResourceID<WTexture> id) const;
		WBuffer* GetResource(ResourceID<WBuffer> id) const;
		WSampler* GetResource(ResourceID<WSampler> id) const;

	private:
		std::vector<WTexture*> m_vpTextures;
		std::vector<WBuffer*> m_vpBuffers;
		std::vector<WSampler*> m_vpSamplers;
		WResourceManager();
		~WResourceManager();
	};
}

