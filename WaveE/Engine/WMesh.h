#pragma once
#include "WBuffer.h"
#include "WResource.h"

namespace WaveE
{
	struct DefaultVertex
	{
		wma::vec3 position;
		wma::vec3 normal;
		wma::vec2 texCoord;
	};

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

		Topology topology{ TRIANGLE_LIST };
		void* pVertexData{ nullptr };
		UINT vertexCount{ 0 };
		size_t vertexStrideBytes{ sizeof(DefaultVertex) };
		void* pIndexData{ nullptr };
		UINT indexCount{ 0 };
	};

	class WMesh
	{
	public:
		WMesh(const WMeshDescriptor& rDescriptor);
		~WMesh();

		void Bind() const;
		UINT GetVertexCount() const { return m_vertexCount; }
		UINT GetIndexCount() const { return m_IndexCount; }
		size_t GetVertexStride() const { return m_vertexStrideBytes; }
		ResourceID<WBuffer> GetVertexBufferID() const { return m_vertexBufferID; }
		ResourceID<WBuffer> GetIndexBufferID() const { return m_indexBufferID; }

		bool IsIndexed() const { return m_isIndexed; }
		bool IsTriangleList() const { return m_topology == D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST; }
	private:
		ResourceID<WBuffer> m_vertexBufferID{};
		ResourceID<WBuffer> m_indexBufferID{};

		UINT m_vertexCount;
		UINT m_IndexCount;
		size_t m_vertexStrideBytes;

		D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;
		D3D12_INDEX_BUFFER_VIEW m_indexBufferView;
		D3D_PRIMITIVE_TOPOLOGY m_topology;

		bool m_isIndexed;
	};
}

