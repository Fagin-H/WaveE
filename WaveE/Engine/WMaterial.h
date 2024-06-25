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
		UINT maxBuffers{ 1 };
		WTextureDescriptor* textureDescriptorArray{ nullptr };
		UINT numTextures{ 0 };
		UINT maxTextures{ 4 };
		WSamplerDescriptor* samplerDescriptorArray{ nullptr };
		UINT numSamplers{ 0 };
		UINT maxSamplers{ 4 };
	};

	class WMaterial
	{
	public:
		WMaterial(const WMaterialDescriptor rDescriptor);

		void SwapBuffer(ResourceID<WBuffer> bufferID, UINT index);
		void SwapTexture(ResourceID<WTexture> textureID, UINT index);

		void BindMaterial();
	private:
		ResourceID<WPipeline> m_pipeline{};
		WDescriptorHeapManager::Allocation m_buffersAndTextures{};
		ResourceBlock<WSampler> m_samplers{};
		UINT m_maxBuffers;
		UINT m_maxTextures;
		UINT m_maxSamplers;
	};
}

