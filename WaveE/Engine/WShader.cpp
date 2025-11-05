#include "stdafx.h"
#include "WShader.h"
#include "WaveManager.h"

namespace WaveE
{
	WShader::WShader(const WShaderDescriptor& rDescriptor)
		: m_type{ rDescriptor.type }
	{
        WaveEDevice pDevice = WaveManager::Instance()->GetDevice();

        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = rDescriptor.shaderData.bytecodeLength;
        createInfo.pCode = reinterpret_cast<const uint32_t*>(rDescriptor.shaderData.pShaderBytecode);

        VkResult result = vkCreateShaderModule(pDevice, &createInfo, nullptr, &m_pShaderModule);
        WAVEE_ASSERT_MESSAGE(result == VK_SUCCESS, "Failed to create Vulkan shader module!");
	}

	WShader::~WShader()
	{
	}
}