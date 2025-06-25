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
		void LoadMeshRT(const std::string& filePath, std::vector<DefaultRTVertex>& vVertices, std::vector<wma::vec3>& vVertexPositions, std::vector<UINT>& vIndices);
		void LoadMeshRaterAndRT(const std::string& filePath, std::vector<DefaultVertex>& vVertices, std::vector<DefaultRTVertex>& vVerticesRT, std::vector<wma::vec3>& vVertexPositions, std::vector<UINT>& vIndices);

	private:
		WMeshLoader();

	};
}

