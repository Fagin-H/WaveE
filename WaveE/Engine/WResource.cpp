#include "stdafx.h"
#include "WResource.h"
#include "WResourceManager.h"

#include "WBuffer.h"
#include "WMaterial.h"
#include "WMesh.h"
#include "WPipeline.h"
#include "WSampler.h"
#include "WShader.h"
#include "WTexture.h"

namespace WaveE
{
	template<typename Resource>
	Resource* GetResourceFromManager(ResourceID<Resource> id)
	{
		return WResourceManager::Instance()->GetResource(id);
	}

	template WBuffer* GetResourceFromManager(ResourceID<WBuffer> id);
	template WMaterial* GetResourceFromManager(ResourceID<WMaterial> id);
	template WMesh* GetResourceFromManager(ResourceID<WMesh> id);
	template WPipeline* GetResourceFromManager(ResourceID<WPipeline> id);
	template WSampler* GetResourceFromManager(ResourceID<WSampler> id);
	template WShader* GetResourceFromManager(ResourceID<WShader> id);
	template WTexture* GetResourceFromManager(ResourceID<WTexture> id);
}