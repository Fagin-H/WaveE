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
			const wma::vec3& scale,
			const wma::vec4& colour
		);

	private:
		WMeshBuilder() = default;
	};
}

