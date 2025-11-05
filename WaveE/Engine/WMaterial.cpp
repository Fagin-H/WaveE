#include "stdafx.h"
#include "WMaterial.h"
#include "WaveManager.h"

namespace WaveE
{
	int WMaterial::m_nextMaterialID{ 0 };

	WMaterial::WMaterial(const WMaterialDescriptor& rDescriptor)
		: m_materialID{ m_nextMaterialID++ }
		, m_pixelShader{ rDescriptor.pixelShader }
		, m_vertexShader{ rDescriptor.vertexShader }
	{
		memcpy(&m_vUniformBuffers[0], &rDescriptor.vUniformBuffers[0], sizeof(ResourceID<WBuffer>) * MAX_MATERIAL_SLOTS);
		memcpy(&m_vStorageBuffers[0], &rDescriptor.vStorageBuffers[0], sizeof(ResourceID<WBuffer>) * MAX_MATERIAL_SLOTS);
		memcpy(&m_vTextures[0], &rDescriptor.vTextures[0], sizeof(ResourceID<WTexture>) * MAX_MATERIAL_SLOTS);
		memcpy(&m_vSamplers[0], &rDescriptor.vSamplers[0], sizeof(ResourceID<WSampler>) * MAX_MATERIAL_SLOTS);
	}

	void WMaterial::SwapBuffer(ResourceID<WBuffer> bufferID, UINT index)
	{
		WAVEE_ASSERT_MESSAGE(index < MAX_MATERIAL_SLOTS, "Buffer index out of range!");
		
		bool isStorage = bufferID.GetResource()->IsStorage();

		if (isStorage)
		{
			m_vStorageBuffers[index] = bufferID;
		}
		else
		{
			m_vUniformBuffers[index] = bufferID;
		}
	}

	void WMaterial::SwapTexture(ResourceID<WTexture> textureID, UINT index)
	{
		WAVEE_ASSERT_MESSAGE(index < MAX_MATERIAL_SLOTS, "Texture index out of range!");

		m_vTextures[index] = textureID;
	}

	void WMaterial::SwapSampler(ResourceID<WSampler> samplerID, UINT index)
	{
		WAVEE_ASSERT_MESSAGE(index < MAX_MATERIAL_SLOTS, "Sampler index out of range!");

		m_vSamplers[index] = samplerID;
	}

	void WMaterial::FillMaterialBuffer(MaterialBuffer& materialBuffer) const
	{
		for (int i = 0; i < MAX_MATERIAL_SLOTS; i++)
		{
			materialBuffer.uniformBuffers[i] = m_vUniformBuffers[i].GetResource()->GetSlot();
			materialBuffer.storageBuffers[i] = m_vStorageBuffers[i].GetResource()->GetSlot();
			materialBuffer.textures[i] = m_vTextures[i].GetResource()->GetSlot();
			materialBuffer.samplers[i] = m_vSamplers[i].GetResource()->GetSlot();
		}
	}
}