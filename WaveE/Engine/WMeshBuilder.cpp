#include "stdafx.h"
#include "WMeshBuilder.h"

namespace WaveE
{
	WAVEE_SINGLETON_CPP(WMeshBuilder)

	void WMeshBuilder::CreateCubeMesh(std::vector<DefaultVertex>& vertices, std::vector<UINT>& indices, const glm::vec3& scale /*= glm::vec3(1.0f)*/, const glm::vec4& colour /*= glm::vec4(1.0f) */)
	{
		vertices.clear();
		indices.clear();
	}
}