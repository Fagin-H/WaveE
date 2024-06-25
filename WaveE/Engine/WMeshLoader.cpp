#include "stdafx.h"

#define  TINYOBJLOADER_IMPLEMENTATION // Implementation for tiny obj loader, must be defined once in the project
#include "WMeshLoader.h"

namespace WaveE
{
	WAVEE_SINGLETON_CPP(WMeshLoader)

	void WMeshLoader::LoadMesh(const std::string& filePath, std::vector<DefaultVertex>& vVertices, std::vector<UINT>& vIndices)
	{
		vVertices.clear();
		vIndices.clear();

		tinyobj::ObjReaderConfig reader_config;

		tinyobj::ObjReader reader;

		WAVEE_ASSERT_MESSAGE(reader.ParseFromFile(filePath, reader_config), "Failes to parse mesh from file!");

		if (!reader.Warning().empty())
		{
			std::cout << "TinyObjReader: " << reader.Warning();
		}

		auto& attrib = reader.GetAttrib();
		auto& shapes = reader.GetShapes();
		auto& materials = reader.GetMaterials();

		// Loop over shapes
		for (size_t s = 0; s < shapes.size(); s++)
		{
			// Loop over faces(polygon)
			size_t index_offset = 0;
			for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++)
			{
				size_t fv = size_t(shapes[s].mesh.num_face_vertices[f]);

				// Loop over vertices in the face.
				for (size_t v = 0; v < fv; v++)
				{
					DefaultVertex newVertex;

					// access to vertex
					tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
					tinyobj::real_t vx = attrib.vertices[3 * size_t(idx.vertex_index) + 0];
					tinyobj::real_t vy = attrib.vertices[3 * size_t(idx.vertex_index) + 1];
					tinyobj::real_t vz = attrib.vertices[3 * size_t(idx.vertex_index) + 2];

					newVertex.position = glm::vec3{ vx, vy, vz };

					// Check if `normal_index` is zero or positive. negative = no normal data
					if (idx.normal_index >= 0)
					{
						tinyobj::real_t nx = attrib.normals[3 * size_t(idx.normal_index) + 0];
						tinyobj::real_t ny = attrib.normals[3 * size_t(idx.normal_index) + 1];
						tinyobj::real_t nz = attrib.normals[3 * size_t(idx.normal_index) + 2];

						newVertex.normal = glm::vec3{ nx, ny, nz };
					}

					// Check if `texcoord_index` is zero or positive. negative = no texcoord data
					if (idx.texcoord_index >= 0)
					{
						tinyobj::real_t tx = attrib.texcoords[2 * size_t(idx.texcoord_index) + 0];
						tinyobj::real_t ty = attrib.texcoords[2 * size_t(idx.texcoord_index) + 1];

						newVertex.texCoord = glm::vec2{ tx, ty };
					}

					vVertices.push_back(newVertex);
				}
				index_offset += fv;
			}
		}
	}

	WMeshLoader::WMeshLoader()
	{

	}

}