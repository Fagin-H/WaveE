#include "stdafx.h"
#include "WResourceManager.h"
#include "WaveManager.h"
#include <fstream>
#include <filesystem>

namespace WaveE
{
	WAVEE_SINGLETON_CPP(WResourceManager)

	WResourceManager::WResourceManager()
	{

	}

	WResourceManager::~WResourceManager()
	{
		for (WTexture* pTexture : m_vpTextures)
		{
			if (pTexture)
			{
				delete pTexture;
			}
		}

		for (WBuffer* pBuffer : m_vpBuffers)
		{
			if (pBuffer)
			{
				delete pBuffer;
			}
		}

		for (WSampler* pSampler : m_vpSamplers)
		{
			if (pSampler)
			{
				delete pSampler;
			}
		}

		for (WMesh* pMesh : m_vpMeshes)
		{
			if (pMesh)
			{
				delete pMesh;
			}
		}

		for (WShader* pShader : m_vpShaders)
		{
			if (pShader)
			{
				delete pShader;
			}
		}
	}

	ResourceID<WTexture> WResourceManager::CreateResource(const WTextureDescriptor& rDescriptor, WDescriptorHeapManager::Allocation allocation /*= WDescriptorHeapManager::InvalidAllocation()*/, UINT offset /*= 0*/)
	{
		m_vpTextures.push_back(new WTexture{ rDescriptor, allocation, offset });
		return ResourceID<WTexture>{static_cast<UINT>(m_vpTextures.size()) - 1};
	}

	ResourceID<WBuffer> WResourceManager::CreateResource(const WBufferDescriptor& rDescriptor, WDescriptorHeapManager::Allocation allocation /*= WDescriptorHeapManager::InvalidAllocation()*/, UINT offset /*= 0*/)
	{
		m_vpBuffers.push_back(new WBuffer{ rDescriptor, allocation, offset });
		return ResourceID<WBuffer>{static_cast<UINT>(m_vpBuffers.size()) - 1};
	}

	ResourceID<WSampler> WResourceManager::CreateResource(const WSamplerDescriptor& rDescriptor, WDescriptorHeapManager::Allocation allocation /*= WDescriptorHeapManager::InvalidAllocation()*/, UINT offset /*= 0*/)
	{
		m_vpSamplers.push_back(new WSampler{ rDescriptor, allocation, offset });
		return ResourceID<WSampler>{static_cast<UINT>(m_vpSamplers.size()) - 1};
	}

	ResourceID<WMesh> WResourceManager::CreateResource(const WMeshDescriptor& rDescriptor)
	{
		m_vpMeshes.push_back(new WMesh{ rDescriptor });
		return ResourceID<WMesh>{static_cast<UINT>(m_vpMeshes.size()) - 1};
	}

	ResourceID<WShader> WResourceManager::CreateResource(const WShaderDescriptor& rDescriptor)
	{
		m_vpShaders.push_back(new WShader{ rDescriptor });
		return ResourceID<WShader>{static_cast<UINT>(m_vpShaders.size()) - 1};
	}

	ResourceID<WPipeline> WResourceManager::CreateResource(const WPipelineDescriptor& rDescriptor)
	{
		m_vpPipelines.push_back(new WPipeline{ rDescriptor });
		return ResourceID<WPipeline>{static_cast<UINT>(m_vpPipelines.size()) - 1};
	}

	ResourceBlock<WTexture> WResourceManager::CreateResourceBlock(WTextureDescriptor* pDescriptors, UINT numDescriptors)
	{
		WAVEE_ASSERT_MESSAGE(numDescriptors > 0, "Can't create resource block of size 0!");
		WAVEE_ASSERT_MESSAGE(pDescriptors, "Descriptors not provided!");

		ResourceBlock<WTexture> resourceBlock;
		resourceBlock.numElements = numDescriptors;
		resourceBlock.startID = m_vpTextures.size();
		resourceBlock.allocation = WaveManager::Instance()->GetCBV_SRV_UAVHeap()->Allocate(numDescriptors);

		for (UINT i = 0; i < numDescriptors; i++)
		{
			const WTextureDescriptor& rDescriptor = pDescriptors[i];
			CreateResource(rDescriptor, resourceBlock.allocation, i);
		}

		return resourceBlock;
	}

	ResourceBlock<WBuffer> WResourceManager::CreateResourceBlock(WBufferDescriptor* pDescriptors, UINT numDescriptors)
	{
		WAVEE_ASSERT_MESSAGE(numDescriptors > 0, "Can't create resource block of size 0!");
		WAVEE_ASSERT_MESSAGE(pDescriptors, "Descriptors not provided!");

		ResourceBlock<WBuffer> resourceBlock;
		resourceBlock.numElements = numDescriptors;
		resourceBlock.startID = m_vpBuffers.size();
		resourceBlock.allocation = WaveManager::Instance()->GetCBV_SRV_UAVHeap()->Allocate(numDescriptors);

		for (UINT i = 0; i < numDescriptors; i++)
		{
			const WBufferDescriptor& rDescriptor = pDescriptors[i];
			CreateResource(rDescriptor, resourceBlock.allocation, i);
		}

		return resourceBlock;
	}

	ResourceBlock<WSampler> WResourceManager::CreateResourceBlock(WSamplerDescriptor* pDescriptors, UINT numDescriptors)
	{
		WAVEE_ASSERT_MESSAGE(numDescriptors > 0, "Can't create resource block of size 0!");
		WAVEE_ASSERT_MESSAGE(pDescriptors, "Descriptors not provided!");

		ResourceBlock<WSampler> resourceBlock;
		resourceBlock.numElements = numDescriptors;
		resourceBlock.startID = m_vpSamplers.size();
		resourceBlock.allocation = WaveManager::Instance()->GetSamplerHeap()->Allocate(numDescriptors);

		for (UINT i = 0; i < numDescriptors; i++)
		{
			const WSamplerDescriptor& rDescriptor = pDescriptors[i];
			CreateResource(rDescriptor, resourceBlock.allocation, i);
		}

		return resourceBlock;
	}

	WTexture* WResourceManager::GetResource(ResourceID<WTexture> id) const
	{
		WAVEE_ASSERT_MESSAGE(id.id < m_vpTextures.size(), "Texture ID out of range!");

		return m_vpTextures[id.id];
	}

	WBuffer* WResourceManager::GetResource(ResourceID<WBuffer> id) const
	{
		WAVEE_ASSERT_MESSAGE(id.id < m_vpBuffers.size(), "Buffer ID out of range!");

		return m_vpBuffers[id.id];
	}

	WSampler* WResourceManager::GetResource(ResourceID<WSampler> id) const
	{
		WAVEE_ASSERT_MESSAGE(id.id < m_vpSamplers.size(), "Sampler ID out of range!");

		return m_vpSamplers[id.id];
	}

	WMesh* WResourceManager::GetResource(ResourceID<WMesh> id) const
	{
		WAVEE_ASSERT_MESSAGE(id.id < m_vpMeshes.size(), "Mesh ID out of range!");

		return m_vpMeshes[id.id];
	}

	WShader* WResourceManager::GetResource(ResourceID<WShader> id) const
	{
		WAVEE_ASSERT_MESSAGE(id.id < m_vpShaders.size(), "Shader ID out of range!");

		return m_vpShaders[id.id];
	}

	WPipeline* WResourceManager::GetResource(ResourceID<WPipeline> id) const
	{
		WAVEE_ASSERT_MESSAGE(id.id < m_vpPipelines.size(), "Pipeline ID out of range!");

		return m_vpPipelines[id.id];
	}

	WShader* WResourceManager::GetShader(const std::string& shaderName) const
	{
		auto it = m_shaderIndexMap.find(shaderName);
		WAVEE_ASSERT_MESSAGE(it != m_shaderIndexMap.end(), "Shader not found!");
		WAVEE_ASSERT_MESSAGE(it->second < m_vpShaders.size(), "Shader index out of range!");

		if (it != m_shaderIndexMap.end())
		{
			return m_vpShaders[it->second];
		}

		return nullptr;
	}

	void WResourceManager::LoadShadersFromDirectory(const std::string& directoryPath)
	{
		namespace fs = std::filesystem;

		for (const auto& entry : fs::recursive_directory_iterator(directoryPath))
		{
			if (entry.is_regular_file())
			{
				const std::string& filepath = entry.path().string();
				const std::string& filename = entry.path().stem().string();
				const std::string& extension = entry.path().extension().string();

				if (extension == ".cso")
				{
					if (filename.find("_VS") != std::string::npos)
					{
						m_shaderIndexMap[filename] = LoadShader(filepath, WShaderDescriptor::Vertex);
					}
					else if (filename.find("_PS") != std::string::npos)
					{
						m_shaderIndexMap[filename] = LoadShader(filepath, WShaderDescriptor::Pixel);
					}
					else if (filename.find("_CS") != std::string::npos)
					{
						m_shaderIndexMap[filename] = LoadShader(filepath, WShaderDescriptor::Compute);
					}
				}
			}
		}
	}

	UINT WResourceManager::LoadShader(const std::string& filepath, WShaderDescriptor::ShaderType type)
	{
		std::ifstream shaderFile{ filepath, std::ios::binary};
		WAVEE_ASSERT_MESSAGE(shaderFile.is_open(), "Could not open shader file!");

		shaderFile.seekg(0, shaderFile.end);
		size_t fileSize = static_cast<size_t>(shaderFile.tellg());
		shaderFile.seekg(0, shaderFile.beg);

		char* shaderBytecode = new char[fileSize];
		shaderFile.read(shaderBytecode, fileSize);
		shaderFile.close();

		WShaderDescriptor descriptor;
		descriptor.type = type;
		descriptor.shaderData.pShaderBytecode = shaderBytecode;
		descriptor.shaderData.bytecodeLength = fileSize;

		UINT shaderID = CreateResource(descriptor).id;

		delete shaderBytecode;

		return shaderID;
	}

}