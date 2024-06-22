#pragma once
#include "WTexture.h"
#include "WBuffer.h"
#include "WSampler.h"
#include "WMesh.h"
#include "WShader.h"
#include "WResource.h"
#include <string>
#include <unordered_map>

namespace WaveE
{
	template<typename Resource>
	struct ResourceBlock
	{
		UINT startID;
		UINT numElements;
		WDescriptorHeapManager::Allocation allocation;

		ResourceID<Resource> GetResorce(UINT index)
		{
			WAVEE_ASSERT_MESSAGE(index < numElements, "Resource index out of range!");
			return ResourceID<Resource>{startID + index};
		}
	};

	// A class to manage resources
	// #TODO Currently no way to release resources
	class WResourceManager
	{
		WAVEE_SINGLETON(WResourceManager)

	public:
		ResourceID<WTexture> CreateResource(const WTextureDescriptor& rDescriptor, WDescriptorHeapManager::Allocation allocation = WDescriptorHeapManager::InvalidAllocation(), UINT offset = 0);
		ResourceID<WBuffer> CreateResource(const WBufferDescriptor& rDescriptor, WDescriptorHeapManager::Allocation allocation = WDescriptorHeapManager::InvalidAllocation(), UINT offset = 0);
		ResourceID<WSampler> CreateResource(const WSamplerDescriptor& rDescriptor, WDescriptorHeapManager::Allocation allocation = WDescriptorHeapManager::InvalidAllocation(), UINT offset = 0);
		ResourceID<WMesh> CreateResource(const WMeshDescriptor& rDescriptor);
		ResourceID<WShader> CreateResource(const WShaderDescriptor& rDescriptor);

		ResourceBlock<WTexture> CreateResourceBlock(WTextureDescriptor* pDescriptors, UINT numDescriptors);
		ResourceBlock<WBuffer> CreateResourceBlock(WBufferDescriptor* pDescriptors, UINT numDescriptors);
		ResourceBlock<WSampler> CreateResourceBlock(WSamplerDescriptor* pDescriptors, UINT numDescriptors);

		WTexture* GetResource(ResourceID<WTexture> id) const;
		WBuffer* GetResource(ResourceID<WBuffer> id) const;
		WSampler* GetResource(ResourceID<WSampler> id) const;
		WMesh* GetResource(ResourceID<WMesh> id) const;
		WShader* GetResource(ResourceID<WShader> id) const;
		WShader* GetShader(const std::string& shaderName) const;

		void LoadShadersFromDirectory(const std::string& directoryPath);

	private:
		std::vector<WTexture*> m_vpTextures;
		std::vector<WBuffer*> m_vpBuffers;
		std::vector<WSampler*> m_vpSamplers;
		std::vector<WMesh*> m_vpMeshes;
		std::vector<WShader*> m_vpShaders;

		std::unordered_map<std::string, UINT> m_shaderIndexMap;

		UINT LoadShader(const std::string& filepath, WShaderDescriptor::ShaderType type);

		WResourceManager();
		~WResourceManager();
	};
}

