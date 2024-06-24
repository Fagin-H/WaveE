#pragma once
#include "WResource.h"
#include "WPipeline.h"
#include "WBuffer.h"
#include "WTexture.h"
#include "WSampler.h"

namespace WaveE
{
	struct WMaterialDescriptor
	{
		ResourceID<WPipeline> pipeline{};
		WBufferDescriptor* bufferDescriptorArray{ nullptr };
		UINT numBuffers{ 0 };
		WTextureDescriptor* textureDescriptorArray{ nullptr };
		UINT numTextures{ 0 };
		WSamplerDescriptor* samplerDescriptorArray{ nullptr };
		UINT numSamplers{ 0 };
	};

	class WMaterial
	{
	public:
		WMaterial(const WMaterialDescriptor rDescriptor);

		void BindMaterial();
	private:
		ResourceID<WPipeline> m_pipeline{};
		WDescriptorHeapManager::Allocation m_buffersAndTextures{};
		ResourceBlock<WSampler> m_samplers{};
	};
}

