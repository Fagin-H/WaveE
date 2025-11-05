#pragma once
#include "WResourceManager.h"

namespace WaveE 
{
	class WDescriptorManager
	{
	public:
		WAVEE_NO_COPY(WDescriptorManager);

		WDescriptorManager();
		void Init(int maxTextures, int maxUniformBuffers, int maxStorageBuffers, int maxSamplers, int reservedTextures, int reservedUniformBuffers, int reservedStorageBuffers, int reservedSamplers);

		UINT AddResource(ResourceID<WTexture> textureID, int slot = -1);
		UINT AddResource(ResourceID<WBuffer> bufferID, int slot = -1);
		UINT AddResource(ResourceID<WSampler> samplerID, int slot = -1);

		UINT AddResource(VkBuffer buffer, bool isStorage, int slot = -1);
		UINT AddResource(VkImageView texture, int slot = -1);
		UINT AddResource(VkSampler sampler, int slot = -1);

		UINT GetBinding(ResourceID<WTexture> textureID) const;
		UINT GetBinding(ResourceID<WBuffer> bufferID) const;
		UINT GetBinding(ResourceID<WSampler> samplerID) const;

	private:
		void AddResource(VkDescriptorImageInfo* pImageInfo, VkDescriptorBufferInfo* pBufferInfo, VkDescriptorType type, int binding, int slot);

		int m_maxTextures;
		int m_maxUniformBuffers;
		int m_maxStorageBuffers;
		int m_maxSamplers;

		UINT m_nextTextureSlot{ 0 };
		UINT m_nextUniformBufferSlot{ 0 };
		UINT m_nextStorageBufferSlot{ 0 };
		UINT m_nextSamplerSlot{ 0 };

		VkDescriptorSetLayout m_pDescriptorSetLayout{ nullptr };
		VkDescriptorPool m_pDescriptorPool{ nullptr };
		VkDescriptorSet m_pDescriptorSet{ nullptr };
	};
}

