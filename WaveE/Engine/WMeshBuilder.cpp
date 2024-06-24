#include "stdafx.h"
#include "WMeshBuilder.h"

namespace WaveE
{
	WAVEE_SINGLETON_CPP(WMeshBuilder)

	void WMeshBuilder::CreateCubeMesh(std::vector<DefaultVertex>& vertices, std::vector<UINT>& indices, const glm::vec3& scale /*= glm::vec3(1.0f)*/, const glm::vec4& colour /*= glm::vec4(1.0f) */)
	{
		vertices.clear();
		indices.clear();

        // Define the vertices for each face of the cube
        DefaultVertex faceVertices[6][4] = {
            // Front face
            {
                {glm::vec3{-1.0f, -1.0f, -1.0f} * scale, {0.0f, 0.0f, -1.0f}, colour},
                {glm::vec3{1.0f, -1.0f, -1.0f} *scale, {0.0f, 0.0f, -1.0f}, colour},
                {glm::vec3{1.0f, 1.0f, -1.0f} *scale, {0.0f, 0.0f, -1.0f}, colour},
                {glm::vec3{-1.0f, 1.0f, -1.0f} *scale, {0.0f, 0.0f, -1.0f}, colour}
            },
            // Back face
            {
                {glm::vec3{-1.0f, -1.0f, 1.0f} *scale, {0.0f, 0.0f, 1.0f}, colour},
                {glm::vec3{1.0f, -1.0f, 1.0f} *scale, {0.0f, 0.0f, 1.0f}, colour},
                {glm::vec3{1.0f, 1.0f, 1.0f} *scale, {0.0f, 0.0f, 1.0f}, colour},
                {glm::vec3{-1.0f, 1.0f, 1.0f} *scale, {0.0f, 0.0f, 1.0f}, colour}
            },
            // Left face
            {
                {glm::vec3{-1.0f, -1.0f, -1.0f} *scale, {-1.0f, 0.0f, 0.0f}, colour},
                {glm::vec3{-1.0f, -1.0f, 1.0f} *scale, {-1.0f, 0.0f, 0.0f}, colour},
                {glm::vec3{-1.0f, 1.0f, 1.0f} *scale, {-1.0f, 0.0f, 0.0f}, colour},
                {glm::vec3{-1.0f, 1.0f, -1.0f} *scale, {-1.0f, 0.0f, 0.0f}, colour}
            },
            // Right face
            {
                {glm::vec3{1.0f, -1.0f, -1.0f} *scale, {1.0f, 0.0f, 0.0f}, colour},
                {glm::vec3{1.0f, -1.0f, 1.0f} *scale, {1.0f, 0.0f, 0.0f}, colour},
                {glm::vec3{1.0f, 1.0f, 1.0f} *scale, {1.0f, 0.0f, 0.0f}, colour},
                {glm::vec3{1.0f, 1.0f, -1.0f} *scale, {1.0f, 0.0f, 0.0f}, colour}
            },
            // Top face
            {
                {glm::vec3{-1.0f, 1.0f, -1.0f} *scale, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}},
                {glm::vec3{1.0f, 1.0f, -1.0f} *scale, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 0.0f, 1.0f}},
                {glm::vec3{1.0f, 1.0f, 1.0f} *scale, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f, 1.0f}},
                {glm::vec3{-1.0f, 1.0f, 1.0f} *scale, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f, 0.0f, 1.0f}}
            },
            // Bottom face
            {
                {glm::vec3{-1.0f, -1.0f, -1.0f} *scale, {0.0f, -1.0f, 0.0f}, {1.0f, 0.0f, 1.0f, 1.0f}},
                {glm::vec3{1.0f, -1.0f, -1.0f} *scale, {0.0f, -1.0f, 0.0f}, {0.0f, 1.0f, 1.0f, 1.0f}},
                {glm::vec3{1.0f, -1.0f, 1.0f} *scale, {0.0f, -1.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}},
                {glm::vec3{-1.0f, -1.0f, 1.0f} *scale, {0.0f, -1.0f, 0.0f}, {0.5f, 0.5f, 0.5f, 1.0f}}
            }
        };

        // Define the indices for each face of the cube (two triangles per face)
        uint32_t faceIndices[6][6] = {
            { 0, 1, 2, 0, 2, 3 },  // Front face
            { 4, 5, 6, 4, 6, 7 },  // Back face
            { 8, 9, 10, 8, 10, 11 },  // Left face
            { 12, 13, 14, 12, 14, 15 },  // Right face
            { 16, 17, 18, 16, 18, 19 },  // Top face
            { 20, 21, 22, 20, 22, 23 }   // Bottom face
        };

        // Push the vertices and indices into the vectors
        for (int face = 0; face < 6; ++face)
        {
            for (int i = 0; i < 4; ++i)
            {
                vertices.push_back(faceVertices[face][i]);
            }
            for (int i = 0; i < 6; ++i)
            {
                indices.push_back(faceIndices[face][i] + face * 4);
            }
        }
	}
}