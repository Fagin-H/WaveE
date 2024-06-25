#pragma once
#include "WMesh.h"
#include "TinyObjLoader/tiny_obj_loader.h";

namespace WaveE
{
	class WMeshLoader
	{
		WAVEE_SINGLETON(WMeshLoader)
	public:
		void LoadMesh(const std::string& filePath, std::vector<DefaultVertex>& vVertices, std::vector<UINT>& vIndices);

	private:
		WMeshLoader();

	};
}

