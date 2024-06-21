#include "stdafx.h"
#include "WMesh.h"
#include "WResourceManager.h"
#include "WaveManager.h"

namespace WaveE
{
	D3D_PRIMITIVE_TOPOLOGY GetDX12Topology(WMeshDescriptor::Topology topology)
	{
		switch (topology)
		{
		case WMeshDescriptor::TRIANGLE_LIST: return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		case WMeshDescriptor::TRIANGLE_STRIP: return D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
		case WMeshDescriptor::LINE_LIST: return D3D_PRIMITIVE_TOPOLOGY_LINELIST;
		case WMeshDescriptor::LINE_STRIP: return D3D_PRIMITIVE_TOPOLOGY_LINESTRIP;
		case WMeshDescriptor::POINT_LIST: return D3D_PRIMITIVE_TOPOLOGY_POINTLIST;
		default: return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		}
	}

	WMesh::WMesh(const WMeshDescriptor& rDescriptor)
	{
		WAVEE_ASSERT_MESSAGE(rDescriptor.pVertexData, "No vertex data for mesh!");

		m_topology = GetDX12Topology(rDescriptor.topology);

		m_isIndexed = !rDescriptor.pIndexData;

		WaveEDevice* pDevice = WaveManager::Instance()->GetDevice();

		// Create vertex buffer
		{
			WBufferDescriptor vertexBufferDescriptor = {};
			vertexBufferDescriptor.isDynamic = false;
			vertexBufferDescriptor.m_sizeBytes = rDescriptor.vertexDataSizeBytes;
			vertexBufferDescriptor.type = WBufferDescriptor::Vertex;
			vertexBufferDescriptor.pInitalData = rDescriptor.pVertexData;

			m_vertexBufferID = WResourceManager::Instance()->CreateResource(vertexBufferDescriptor);

			m_vertexBufferView.BufferLocation = m_vertexBufferID.GetResource()->GetBuffer()->GetGPUVirtualAddress();
			m_vertexBufferView.SizeInBytes = rDescriptor.vertexDataSizeBytes;
			m_vertexBufferView.StrideInBytes = rDescriptor.vertexStrideBytes;
		}

		// Create index buffer if needed
		if (m_isIndexed)
		{
			WBufferDescriptor indexBufferDescriptor = {};
			indexBufferDescriptor.isDynamic = false;
			indexBufferDescriptor.m_sizeBytes = rDescriptor.indexDataSizeBytes;
			indexBufferDescriptor.type = WBufferDescriptor::Index;
			indexBufferDescriptor.pInitalData = rDescriptor.pIndexData;

			m_indexBufferID = WResourceManager::Instance()->CreateResource(indexBufferDescriptor);

			m_indexBufferView.BufferLocation = m_indexBufferID.GetResource()->GetBuffer()->GetGPUVirtualAddress();
			m_indexBufferView.SizeInBytes = rDescriptor.indexDataSizeBytes;
			m_indexBufferView.Format = DXGI_FORMAT_R32_UINT;
		}
	}

	WMesh::~WMesh()
	{

	}

	void WMesh::Bind() const
	{
		WaveECommandList* pcommandList = WaveManager::Instance()->GetCommandList();

		pcommandList->IASetPrimitiveTopology(m_topology);
		pcommandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
		
		if (m_isIndexed)
		{
			pcommandList->IASetIndexBuffer(&m_indexBufferView);
		}
	}

}