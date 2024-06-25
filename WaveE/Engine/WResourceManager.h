#pragma once
#include "WTexture.h"
#include "WBuffer.h"
#include "WSampler.h"
#include "WMesh.h"
#include "WShader.h"
#include "WPipeline.h"
#include "WResource.h"
#include "WMaterial.h"
#include <string>
#include <unordered_map>

namespace WaveE
{
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
		ResourceID<WPipeline> CreateResource(const WPipelineDescriptor& rDescriptor);
		ResourceID<WMaterial> CreateResource(const WMaterialDescriptor& rDescriptor);

		ResourceBlock<WTexture> CreateResourceBlock(WTextureDescriptor* pDescriptors, UINT numDescriptors);
		ResourceBlock<WBuffer> CreateResourceBlock(WBufferDescriptor* pDescriptors, UINT numDescriptors);
		ResourceBlock<WSampler> CreateResourceBlock(WSamplerDescriptor* pDescriptors, UINT numDescriptors);

		WTexture* GetResource(ResourceID<WTexture> id) const;
		WTexture* GetTexture(const std::string& textureName) const;
		WBuffer* GetResource(ResourceID<WBuffer> id) const;
		WSampler* GetResource(ResourceID<WSampler> id) const;
		WMesh* GetResource(ResourceID<WMesh> id) const;
		WMesh* GetMesh(const std::string& meshName) const;
		WShader* GetResource(ResourceID<WShader> id) const;
		WShader* GetShader(const std::string& shaderName) const;
		WPipeline* GetResource(ResourceID<WPipeline> id) const;
		WMaterial* GetResource(ResourceID<WMaterial> id) const;


		ResourceID<WTexture> GetTextureID(const std::string& textureName) const;
		ResourceID<WMesh> GetMeshID(const std::string& meshName) const;
		ResourceID<WShader> GetShaderID(const std::string& shaderName) const;

		void LoadShadersFromDirectory(const std::string& directoryPath);
		void LoadShadersFromDirectory(const char* directoryPath);

		void LoadTexturesFromDirectory(const std::string& directoryPath);
		void LoadTexturesFromDirectory(const char* directoryPath);

		void LoadMeshesFromDirectory(const std::string& directoryPath);
		void LoadMeshesFromDirectory(const char* directoryPath);

	private:
		std::vector<WTexture*> m_vpTextures;
		std::vector<WBuffer*> m_vpBuffers;
		std::vector<WSampler*> m_vpSamplers;
		std::vector<WMesh*> m_vpMeshes;
		std::vector<WShader*> m_vpShaders;
		std::vector<WPipeline*> m_vpPipelines;
		std::vector<WMaterial*> m_vpMaterials;

		std::unordered_map<std::string, UINT> m_shaderIndexMap;
		std::unordered_map<std::string, UINT> m_textureIndexMap;
		std::unordered_map<std::string, UINT> m_meshIndexMap;

		UINT LoadShader(const char* filepath, WShaderDescriptor::ShaderType type);
		UINT LoadTexture(const char* filepath, WTextureDescriptor::Format format);
		UINT LoadMesh(const char* filepath);

		WResourceManager();
		~WResourceManager();
	};
}

