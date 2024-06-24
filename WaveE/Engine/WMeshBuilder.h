#pragma once
#include "WMesh.h"

namespace WaveE
{
	class WMeshBuilder
	{
		WAVEE_SINGLETON(WMeshBuilder)
	public:
		static void CreateCubeMesh(
			std::vector<DefaultVertex>& vertices,
			std::vector<UINT>& indices,
			const glm::vec3& scale = glm::vec3(1.0f),
			const glm::vec4& colour = glm::vec4(1.0f)
		);

	private:
		WMeshBuilder() = default;
	};
}

