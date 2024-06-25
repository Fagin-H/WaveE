#include "stdafx.h"
#include "WMaterial.h"
#include "WaveManager.h"

namespace WaveE
{
	WMaterial::WMaterial(const WMaterialDescriptor rDescriptor)
		: m_maxBuffers{rDescriptor.maxBuffers}
		, m_maxTextures{rDescriptor.maxTextures}
		, m_maxSamplers{rDescriptor.maxSamplers}
	{
		WAVEE_ASSERT_MESSAGE(rDescriptor.numBuffers <= rDescriptor.maxBuffers, "Too many buffers!");
		WAVEE_ASSERT_MESSAGE(rDescriptor.numTextures <= rDescriptor.maxTextures, "Too many textures!");
		WAVEE_ASSERT_MESSAGE(rDescriptor.numSamplers <= rDescriptor.maxSamplers, "Too many samplers!");

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
		UINT totalMaxBuffersAndTextures = rDescriptor.maxBuffers + rDescriptor.maxTextures;

		if (totalMaxBuffersAndTextures > 0)
		{
			m_buffersAndTextures = WaveManager::Instance()->GetCBV_SRV_UAVHeap()->Allocate(totalMaxBuffersAndTextures);

			for (UINT i = 0; i < rDescriptor.numBuffers; i++)
			{
				WResourceManager::Instance()->CreateResource(rDescriptor.bufferDescriptorArray[i], m_buffersAndTextures, i);
			}
			for (UINT i = 0; i < rDescriptor.numTextures; i++)
			{
				WResourceManager::Instance()->CreateResource(rDescriptor.textureDescriptorArray[i], m_buffersAndTextures, i + rDescriptor.maxBuffers);
			}
		}

		// Samplers
		if (rDescriptor.numSamplers > 0)
		{
			m_samplers = WResourceManager::Instance()->CreateResourceBlock(rDescriptor.samplerDescriptorArray, rDescriptor.numSamplers);
		}
	}

	void WMaterial::SwapBuffer(ResourceID<WBuffer> bufferID, UINT index)
	{
		WAVEE_ASSERT_MESSAGE(index < m_maxBuffers, "Buffer index out of range!");

		WaveEDevice* pDevice = WaveManager::Instance()->GetDevice();

		D3D12_CONSTANT_BUFFER_VIEW_DESC bufferViewDescriptor;
		bufferViewDescriptor.BufferLocation = bufferID.GetResource()->GetBuffer()->GetGPUVirtualAddress();
		bufferViewDescriptor.SizeInBytes = bufferID.GetResource()->GetBuffer()->GetDesc().Width;

		WDescriptorHeapManager* pHeap = WaveManager::Instance()->GetCBV_SRV_UAVHeap();

		pDevice->CreateConstantBufferView(&bufferViewDescriptor, pHeap->GetCPUHandle(m_buffersAndTextures.index + index));
	}

	void WMaterial::SwapTexture(ResourceID<WTexture> textureID, UINT index)
	{
		WAVEE_ASSERT_MESSAGE(index < m_maxTextures, "Texture index out of range!");

		// Textures are after buffers in same heap
		index += m_maxBuffers;

		WaveEDevice* pDevice = WaveManager::Instance()->GetDevice();

		WDescriptorHeapManager* pHeap = WaveManager::Instance()->GetCBV_SRV_UAVHeap();

		pDevice->CreateShaderResourceView(textureID.GetResource()->GetTexture(), nullptr, pHeap->GetCPUHandle(m_buffersAndTextures.index + index));
	}

	void WMaterial::BindMaterial()
	{
		WaveManager::Instance()->SetPipelineState(m_pipeline);
		
		if (m_samplers.IsValid())
		{
			WaveManager::Instance()->BindSamplers(m_samplers, WaveManager::MATERIAL_SAMPLERS);
		}

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