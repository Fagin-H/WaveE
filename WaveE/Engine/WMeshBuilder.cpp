#include "stdafx.h"
#include "WMeshBuilder.h"

namespace WaveE
{
	WAVEE_SINGLETON_CPP(WMeshBuilder)

	void WMeshBuilder::CreateCubeMesh(std::vector<DefaultVertex>& vertices, std::vector<UINT>& indices, const wma::vec3& scale /*= wma::vec3(1.0f)*/, const wma::vec4& colour /*= wma::vec4(1.0f) */)
	{
		vertices.clear();
		indices.clear();
	}
}