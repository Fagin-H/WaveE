#pragma once
#include "WBuffer.h"
#include "WResource.h"

namespace WaveE
{
	struct WMeshDescriptor
	{
		enum Topology
		{
			TRIANGLE_LIST,
			TRIANGLE_STRIP,
			LINE_LIST,
			LINE_STRIP,
			POINT_LIST
		};

		Topology topology;
		void* pVertexData;
		size_t vertexDataSizeBytes;
		size_t vertexStrideBytes;
		void* pIndexData;
		size_t indexDataSizeBytes;
	};

	class WMesh
	{
	public:
		WMesh(const WMeshDescriptor& rDescriptor);
		~WMesh();

		void Bind() const;

	private:
		ResourceID<WBuffer> m_vertexBufferID{};
		ResourceID<WBuffer> m_indexBufferID{};

		D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;
		D3D12_INDEX_BUFFER_VIEW m_indexBufferView;
		D3D_PRIMITIVE_TOPOLOGY m_topology;

		bool m_isIndexed;
	};
}

