#include "stdafx.h"
#include "WMeshBuilder.h"

namespace WaveE
{
	WAVEE_SINGLETON_CPP(WMeshBuilder)

	void WMeshBuilder::CreateCubeMesh(std::vector<DefaultVertex>& vertices, std::vector<UINT>& indices, const glm::vec3& scale /*= glm::vec3(1.0f)*/, const glm::vec4& colour /*= glm::vec4(1.0f) */)
	{
		vertices.clear();
		indices.clear();

		glm::vec3 positions[] = {
			{-1.0f, -1.0f, -1.0f},
			{ 1.0f, -1.0f, -1.0f},
			{ 1.0f,  1.0f, -1.0f},
			{-1.0f,  1.0f, -1.0f},
			{-1.0f, -1.0f,  1.0f},
			{ 1.0f, -1.0f,  1.0f},
			{ 1.0f,  1.0f,  1.0f},
			{-1.0f,  1.0f,  1.0f}
		};

		glm::vec3 normals[] = {
			{0.0f, 0.0f, -1.0f},
			{0.0f, 0.0f,  1.0f},
			{0.0f, -1.0f, 0.0f},
			{0.0f, 1.0f, 0.0f},
			{-1.0f, 0.0f, 0.0f},
			{1.0f, 0.0f, 0.0f}
		};

		UINT faceIndices[] = {
			0, 1, 2, 2, 3, 0,   // Front face
			4, 5, 6, 6, 7, 4,   // Back face
			0, 1, 5, 5, 4, 0,   // Bottom face
			2, 3, 7, 7, 6, 2,   // Top face
			0, 3, 7, 7, 4, 0,   // Left face
			1, 2, 6, 6, 5, 1    // Right face
		};

		for (UINT i = 0; i < 8; ++i)
		{
			DefaultVertex vertex;
			vertex.position = positions[i] * scale;
			vertex.colour = colour;
			vertex.normal = glm::normalize(positions[i]);
			vertices.push_back(vertex);
		}

		for (UINT i = 0; i < 36; ++i)
		{
			indices.push_back(faceIndices[i]);
		}
	}
}