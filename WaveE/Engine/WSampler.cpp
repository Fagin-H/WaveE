#include "stdafx.h"
#include "WSampler.h"
#include "WaveManager.h"

namespace WaveE
{
	VkFilter ConvertFilter(const WSamplerDescriptor::Filter filter)
	{
		switch (filter)
		{
			case WSamplerDescriptor::Point:       return VK_FILTER_NEAREST;
			case WSamplerDescriptor::Linear:      return VK_FILTER_LINEAR;
			case WSamplerDescriptor::Anisotropic: return VK_FILTER_LINEAR;
			default:                               return VK_FILTER_LINEAR;
		}
	}

	VkSamplerAddressMode ConvertAddressMode(const WSamplerDescriptor::AddressMode addressMode)
	{
		switch (addressMode)
		{
			case WSamplerDescriptor::Wrap:   return VK_SAMPLER_ADDRESS_MODE_REPEAT;
			case WSamplerDescriptor::Clamp:  return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
			case WSamplerDescriptor::Border: return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
			default:                          return VK_SAMPLER_ADDRESS_MODE_REPEAT;
		}
	}

	WSampler::WSampler(const WSamplerDescriptor& rDescriptor)
	{

		WaveEDevice pDevice = WaveManager::Instance()->GetDevice();

		VkSamplerCreateInfo samplerInfo{};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = ConvertFilter(rDescriptor.filter);
		samplerInfo.minFilter = ConvertFilter(rDescriptor.filter);
		samplerInfo.addressModeU = ConvertAddressMode(rDescriptor.addressMode);
		samplerInfo.addressModeV = ConvertAddressMode(rDescriptor.addressMode);
		samplerInfo.addressModeW = ConvertAddressMode(rDescriptor.addressMode);
		samplerInfo.mipLodBias = rDescriptor.mipLODBias;
		samplerInfo.anisotropyEnable = (rDescriptor.filter == WSamplerDescriptor::Anisotropic) ? VK_TRUE : VK_FALSE;
		samplerInfo.maxAnisotropy = rDescriptor.maxAnisotropy;
		samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
		samplerInfo.unnormalizedCoordinates = VK_FALSE;
		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
		samplerInfo.minLod = rDescriptor.minLOD;
		samplerInfo.maxLod = rDescriptor.maxLOD;
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

		VkResult result = vkCreateSampler(pDevice, &samplerInfo, nullptr, &m_pSampler);
		WAVEE_ASSERT_MESSAGE(result == VK_SUCCESS, "Failed to create Vulkan sampler!");

		m_slot = WaveManager::Instance()->GetDescriptorManager()->AddResource(m_pSampler, rDescriptor.descriptorSlot);
	}

	WSampler::~WSampler()
	{
	}
}
