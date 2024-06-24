#include "stdafx.h"
#include "WMaterial.h"
#include "WaveManager.h"

namespace WaveE
{
	WMaterial::WMaterial(const WMaterialDescriptor rDescriptor)
	{
		// Pipeline
		if (rDescriptor.pipeline.IsValid())
		{
			m_pipeline = rDescriptor.pipeline;
		}
		else
		{
			m_pipeline = WaveManager::Instance()->GetDefaultPipelineState();
		}

		// Buffers and textures
		UINT totalBuffersAndTextures = rDescriptor.numBuffers + rDescriptor.numTextures;

		if (totalBuffersAndTextures > 0)
		{
			m_buffersAndTextures = WaveManager::Instance()->GetCBV_SRV_UAVHeap()->Allocate(totalBuffersAndTextures);

			for (UINT i = 0; i < rDescriptor.numBuffers; i++)
			{
				WResourceManager::Instance()->CreateResource(rDescriptor.bufferDescriptorArray[i], m_buffersAndTextures, i);
			}
			for (UINT i = 0; i < rDescriptor.numTextures; i++)
			{
				WResourceManager::Instance()->CreateResource(rDescriptor.textureDescriptorArray[i], m_buffersAndTextures, i + rDescriptor.numBuffers);
			}
		}

		// Samplers
		if (rDescriptor.numSamplers > 0)
		{
			m_samplers = WResourceManager::Instance()->CreateResourceBlock(rDescriptor.samplerDescriptorArray, rDescriptor.numSamplers);
		}
	}

	void WMaterial::BindMaterial()
	{
		WaveManager::Instance()->SetPipelineState(m_pipeline);
		
		if (m_buffersAndTextures.IsValid())
		{
			WaveManager::Instance()->BindResource(m_buffersAndTextures, WaveManager::MATERIAL_CBV_SRV);
		}

		if (m_samplers.IsValid())
		{
			WaveManager::Instance()->BindSamplers(m_samplers, WaveManager::MATERIAL_SAMPLERS);
		}
	}
}