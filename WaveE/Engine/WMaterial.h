#pragma once
#include "WResource.h"
#include "WPipeline.h"
#include "WBuffer.h"
#include "WTexture.h"
#include "WSampler.h"

namespace WaveE
{
	constexpr UINT MAX_MATERIAL_SLOTS{ 4 };

	struct WMaterialDescriptor
	{
		ResourceID<WShader> pixelShader{};
		ResourceID<WShader> vertexShader{};
		ResourceID<WBuffer> vUniformBuffers[MAX_MATERIAL_SLOTS];
		ResourceID<WBuffer> vStorageBuffers[MAX_MATERIAL_SLOTS];
		ResourceID<WTexture> vTextures[MAX_MATERIAL_SLOTS];
		ResourceID<WSampler> vSamplers[MAX_MATERIAL_SLOTS];
	};

	struct MaterialBuffer 
	{
		uint32_t uniformBuffers[MAX_MATERIAL_SLOTS];
		uint32_t storageBuffers[MAX_MATERIAL_SLOTS];
		uint32_t textures[MAX_MATERIAL_SLOTS];
		uint32_t samplers[MAX_MATERIAL_SLOTS];
	};

	class WMaterial
	{
	public:
		WMaterial(const WMaterialDescriptor& rDescriptor);

		void SwapBuffer(ResourceID<WBuffer> bufferID, UINT index);
		void SwapTexture(ResourceID<WTexture> textureID, UINT index);
		void SwapSampler(ResourceID<WSampler> samplerID, UINT index);

		void FillMaterialBuffer(MaterialBuffer& materialBuffer) const;
		int GetMaterialID() const { return m_materialID; }
	private:
		static int m_nextMaterialID;
		int m_materialID;
		ResourceID<WShader> m_pixelShader;
		ResourceID<WShader> m_vertexShader;
		ResourceID<WBuffer> m_vUniformBuffers[MAX_MATERIAL_SLOTS];
		ResourceID<WBuffer> m_vStorageBuffers[MAX_MATERIAL_SLOTS];
		ResourceID<WTexture> m_vTextures[MAX_MATERIAL_SLOTS];
		ResourceID<WSampler> m_vSamplers[MAX_MATERIAL_SLOTS];
	};
}

