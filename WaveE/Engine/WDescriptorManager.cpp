#include "stdafx.h"
#include "WDescriptorManager.h"
#include "WaveManager.h"

namespace WaveE
{
	WDescriptorManager::WDescriptorManager()
	{
	}

	void WDescriptorManager::Init(int maxTextures, int maxUniformBuffers, int maxStorageBuffers, int maxSamplers, int reservedTextures, int reservedUniformBuffers, int reservedStorageBuffers, int reservedSamplers)
	{	
		m_maxTextures = maxTextures;
		m_maxUniformBuffers = maxUniformBuffers;
		m_maxStorageBuffers = maxStorageBuffers;
		m_maxSamplers = maxSamplers;

		m_nextTextureSlot = reservedTextures;
		m_nextUniformBufferSlot = reservedUniformBuffers;
		m_nextStorageBufferSlot = reservedStorageBuffers;
		m_nextSamplerSlot = reservedSamplers;

		WaveEDevice pDevice = WaveManager::Instance()->GetDevice();

		// Create Descriptor set layout
		static const UINT numBindings{ 4 };
		VkDescriptorSetLayoutBinding vBindings[numBindings];
		vBindings[0].binding = 0;
		vBindings[0].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
		vBindings[0].descriptorCount = maxTextures;
		vBindings[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		vBindings[1].binding = 1;
		vBindings[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		vBindings[1].descriptorCount = maxUniformBuffers;
		vBindings[1].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

		vBindings[2].binding = 2;
		vBindings[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		vBindings[2].descriptorCount = maxStorageBuffers;
		vBindings[2].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

		vBindings[3].binding = 3;
		vBindings[3].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
		vBindings[3].descriptorCount = maxSamplers;
		vBindings[3].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		VkDescriptorSetLayoutBindingFlagsCreateInfo bindingFlagsInfo{};
		VkDescriptorBindingFlags vBindingFlags[numBindings];
		vBindingFlags[0] = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT;
		vBindingFlags[1] = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT;
		vBindingFlags[2] = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT;
		vBindingFlags[3] = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT;

		bindingFlagsInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		bindingFlagsInfo.bindingCount = numBindings;
		bindingFlagsInfo.pBindingFlags = &vBindingFlags[0];

		VkDescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = numBindings;
		layoutInfo.pBindings = &vBindings[0];
		layoutInfo.pNext = &bindingFlagsInfo;

		VkResult result = vkCreateDescriptorSetLayout(pDevice, &layoutInfo, nullptr, &m_pDescriptorSetLayout);
		WAVEE_ASSERT_MESSAGE(result == VK_SUCCESS, "Failed to create descriptor set layout!");

		// Create descriptor pool
		VkDescriptorPoolSize vPoolSizes[numBindings];
		vPoolSizes[0].type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
		vPoolSizes[0].descriptorCount = maxTextures;

		vPoolSizes[1].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		vPoolSizes[1].descriptorCount = maxUniformBuffers;

		vPoolSizes[2].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		vPoolSizes[2].descriptorCount = maxStorageBuffers;

		vPoolSizes[3].type = VK_DESCRIPTOR_TYPE_SAMPLER;
		vPoolSizes[3].descriptorCount = maxSamplers;

		VkDescriptorPoolCreateInfo descriptorPoolInfo{};
		descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		descriptorPoolInfo.maxSets = 1;
		descriptorPoolInfo.poolSizeCount = numBindings;
		descriptorPoolInfo.pPoolSizes = &vPoolSizes[0];

		result = vkCreateDescriptorPool(pDevice, &descriptorPoolInfo, nullptr, &m_pDescriptorPool);
		WAVEE_ASSERT_MESSAGE(result == VK_SUCCESS, "Failed to create descriptor pool!");

		// Allocate descriptor set
		VkDescriptorSetAllocateInfo descriptorSetAllocInfo{};
		descriptorSetAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		descriptorSetAllocInfo.descriptorPool = m_pDescriptorPool;
		descriptorSetAllocInfo.descriptorSetCount = 1;
		descriptorSetAllocInfo.pSetLayouts = &m_pDescriptorSetLayout;

		result = vkAllocateDescriptorSets(pDevice, &descriptorSetAllocInfo, &m_pDescriptorSet);
		WAVEE_ASSERT_MESSAGE(result == VK_SUCCESS, "Failed to allocate descriptor set!");
	}
	
	UINT WDescriptorManager::AddResource(ResourceID<WTexture> textureID, int slot)
	{
		return AddResource(textureID.GetResource()->GetView(), slot);
	}

	UINT WDescriptorManager::AddResource(ResourceID<WBuffer> bufferID, int slot)
	{
		return AddResource(bufferID.GetResource()->GetBuffer(), bufferID.GetResource()->IsStorage(), slot);
	}

	UINT WDescriptorManager::AddResource(ResourceID<WSampler> samplerID, int slot)
	{
		return AddResource(samplerID.GetResource()->GetSampler(), slot);
	}

	UINT WDescriptorManager::AddResource(VkBuffer buffer, bool isStorage, int slot = -1)
	{
		UINT& nextBufferSlot = isStorage ? m_nextStorageBufferSlot : m_nextUniformBufferSlot;
		int maxBuffers = isStorage ? m_maxStorageBuffers : m_maxUniformBuffers;
		int binding = isStorage ? 2 : 1;
		VkDescriptorType type = isStorage ? VK_DESCRIPTOR_TYPE_STORAGE_BUFFER : VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

		if (slot < 0) {
			slot = nextBufferSlot++;
		}
		else if (slot >= nextBufferSlot) {
			nextBufferSlot = slot + 1;
		}
		WAVEE_ASSERT_MESSAGE(slot < maxBuffers, "Slot index out of range!");

		VkDescriptorBufferInfo bufferInfo{};
		bufferInfo.buffer = buffer;
		bufferInfo.offset = 0;
		bufferInfo.range = VK_WHOLE_SIZE;

		AddResource(nullptr, &bufferInfo, type, binding, slot);

		return slot;
	}

	UINT WDescriptorManager::AddResource(VkImageView texture, int slot = -1)
	{
		if (slot < 0) {
			slot = m_nextTextureSlot++;
		}
		else if (slot >= m_nextTextureSlot) {
			m_nextTextureSlot = slot + 1;
		}
		WAVEE_ASSERT_MESSAGE(slot < m_maxTextures, "Slot index out of range!");

		VkDescriptorImageInfo imageInfo{};
		imageInfo.imageView = texture;
		imageInfo.sampler = VK_NULL_HANDLE;
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		AddResource(&imageInfo, nullptr, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 0, slot);

		return slot;
	}

	UINT WDescriptorManager::AddResource(VkSampler sampler, int slot = -1)
	{
		if (slot < 0) {
			slot = m_nextSamplerSlot++;
		}
		else if (slot >= m_nextSamplerSlot) {
			m_nextSamplerSlot = slot + 1;
		}
		WAVEE_ASSERT_MESSAGE(slot < m_maxSamplers, "Slot index out of range!");

		VkDescriptorImageInfo samplerInfo{};
		samplerInfo.imageView = VK_NULL_HANDLE;
		samplerInfo.sampler = sampler;

		AddResource(&samplerInfo, nullptr, VK_DESCRIPTOR_TYPE_SAMPLER, 3, slot);

		return slot;
	}

	UINT WDescriptorManager::GetBinding(ResourceID<WTexture> textureID) const
	{
		return 0;
	}

	UINT WDescriptorManager::GetBinding(ResourceID<WBuffer> bufferID) const
	{
		return bufferID.GetResource()->IsStorage() ? 2 : 1;
	}

	UINT WDescriptorManager::GetBinding(ResourceID<WSampler> samplerID) const
	{
		return 3;
	}

	void WDescriptorManager::AddResource(VkDescriptorImageInfo* pImageInfo, VkDescriptorBufferInfo* pBufferInfo, VkDescriptorType type, int binding, int slot)
	{
		WaveEDevice pDevice = WaveManager::Instance()->GetDevice();

		VkWriteDescriptorSet write{};
		write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write.dstSet = m_pDescriptorSet;
		write.dstBinding = binding;
		write.dstArrayElement = slot;
		write.descriptorCount = 1;
		write.descriptorType = type;
		write.pImageInfo = pImageInfo;
		write.pBufferInfo = pBufferInfo;

		vkUpdateDescriptorSets(pDevice, 1, &write, 0, nullptr);
	}
}