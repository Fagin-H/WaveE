#include "stdafx.h"
#include "WResourceManager.h"
#include "WaveManager.h"

namespace WaveE
{
	WAVEE_SINGLETON_CPP(WResourceManager)

	WResourceManager::WResourceManager()
	{

	}

	WResourceManager::~WResourceManager()
	{
		for (WTexture* pTexture : m_vpTextures)
		{
			if (pTexture)
			{
				delete pTexture;
			}
		}

		for (WBuffer* pBuffer : m_vpBuffers)
		{
			if (pBuffer)
			{
				delete pBuffer;
			}
		}

		for (WSampler* pSampler : m_vpSamplers)
		{
			if (pSampler)
			{
				delete pSampler;
			}
		}
	}

	ResourceID<WTexture> WResourceManager::CreateResource(const WTextureDescriptor& rDescriptor, WDescriptorHeapManager::Allocation allocation /*= WDescriptorHeapManager::InvalidAllocation()*/, UINT offset /*= 0*/)
	{
		m_vpTextures.push_back(new WTexture{ rDescriptor, allocation, offset });
		return ResourceID<WTexture>{static_cast<UINT>(m_vpTextures.size()) - 1};
	}

	ResourceID<WBuffer> WResourceManager::CreateResource(const WBufferDescriptor& rDescriptor, WDescriptorHeapManager::Allocation allocation /*= WDescriptorHeapManager::InvalidAllocation()*/, UINT offset /*= 0*/)
	{
		m_vpBuffers.push_back(new WBuffer{ rDescriptor, allocation, offset });
		return ResourceID<WBuffer>{static_cast<UINT>(m_vpBuffers.size()) - 1};
	}

	ResourceID<WSampler> WResourceManager::CreateResource(const WSamplerDescriptor& rDescriptor, WDescriptorHeapManager::Allocation allocation /*= WDescriptorHeapManager::InvalidAllocation()*/, UINT offset /*= 0*/)
	{
		m_vpSamplers.push_back(new WSampler{ rDescriptor, allocation, offset });
		return ResourceID<WSampler>{static_cast<UINT>(m_vpSamplers.size()) - 1};
	}

	ResourceID<WMesh> WResourceManager::CreateResource(const WMeshDescriptor& rDescriptor)
	{
		m_vpMeshes.push_back(new WMesh{ rDescriptor });
		return ResourceID<WMesh>{static_cast<UINT>(m_vpMeshes.size()) - 1};
	}

	ResourceBlock<WTexture> WResourceManager::CreateResourceBlock(WTextureDescriptor* pDescriptors, UINT numDescriptors)
	{
		WAVEE_ASSERT_MESSAGE(numDescriptors > 0, "Can't create resource block of size 0!");
		WAVEE_ASSERT_MESSAGE(pDescriptors, "Descriptors not provided!");

		ResourceBlock<WTexture> resourceBlock;
		resourceBlock.numElements = numDescriptors;
		resourceBlock.startID = m_vpTextures.size();
		resourceBlock.allocation = WaveManager::Instance()->GetCBV_SRV_UAVHeap()->Allocate(numDescriptors);

		for (UINT i = 0; i < numDescriptors; i++)
		{
			const WTextureDescriptor& rDescriptor = pDescriptors[i];
			CreateResource(rDescriptor, resourceBlock.allocation, i);
		}

		return resourceBlock;
	}

	ResourceBlock<WBuffer> WResourceManager::CreateResourceBlock(WBufferDescriptor* pDescriptors, UINT numDescriptors)
	{
		WAVEE_ASSERT_MESSAGE(numDescriptors > 0, "Can't create resource block of size 0!");
		WAVEE_ASSERT_MESSAGE(pDescriptors, "Descriptors not provided!");

		ResourceBlock<WBuffer> resourceBlock;
		resourceBlock.numElements = numDescriptors;
		resourceBlock.startID = m_vpBuffers.size();
		resourceBlock.allocation = WaveManager::Instance()->GetCBV_SRV_UAVHeap()->Allocate(numDescriptors);

		for (UINT i = 0; i < numDescriptors; i++)
		{
			const WBufferDescriptor& rDescriptor = pDescriptors[i];
			CreateResource(rDescriptor, resourceBlock.allocation, i);
		}

		return resourceBlock;
	}

	ResourceBlock<WSampler> WResourceManager::CreateResourceBlock(WSamplerDescriptor* pDescriptors, UINT numDescriptors)
	{
		WAVEE_ASSERT_MESSAGE(numDescriptors > 0, "Can't create resource block of size 0!");
		WAVEE_ASSERT_MESSAGE(pDescriptors, "Descriptors not provided!");

		ResourceBlock<WSampler> resourceBlock;
		resourceBlock.numElements = numDescriptors;
		resourceBlock.startID = m_vpSamplers.size();
		resourceBlock.allocation = WaveManager::Instance()->GetSamplerHeap()->Allocate(numDescriptors);

		for (UINT i = 0; i < numDescriptors; i++)
		{
			const WSamplerDescriptor& rDescriptor = pDescriptors[i];
			CreateResource(rDescriptor, resourceBlock.allocation, i);
		}

		return resourceBlock;
	}

	WTexture* WResourceManager::GetResource(ResourceID<WTexture> id) const
	{
		WAVEE_ASSERT_MESSAGE(id.id < m_vpTextures.size(), "Texture ID out of range!");

		return m_vpTextures[id.id];
	}

	WBuffer* WResourceManager::GetResource(ResourceID<WBuffer> id) const
	{
		WAVEE_ASSERT_MESSAGE(id.id < m_vpBuffers.size(), "Buffer ID out of range!");

		return m_vpBuffers[id.id];
	}

	WSampler* WResourceManager::GetResource(ResourceID<WSampler> id) const
	{
		WAVEE_ASSERT_MESSAGE(id.id < m_vpSamplers.size(), "Sampler ID out of range!");

		return m_vpSamplers[id.id];
	}

	WMesh* WResourceManager::GetResource(ResourceID<WMesh> id) const
	{
		WAVEE_ASSERT_MESSAGE(id.id < m_vpMeshes.size(), "Mesh ID out of range!");

		return m_vpMeshes[id.id];
	}

}