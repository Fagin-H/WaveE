#include "stdafx.h"
#include "WMesh.h"
#include "WResourceManager.h"
#include "WaveManager.h"

namespace WaveE
{
	VkPrimitiveTopology GetVulkanTopology(WMeshDescriptor::Topology topology)
	{
		switch (topology)
		{
			case WMeshDescriptor::TRIANGLE_LIST:  return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
			case WMeshDescriptor::TRIANGLE_STRIP: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
			case WMeshDescriptor::LINE_LIST:      return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
			case WMeshDescriptor::LINE_STRIP:     return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
			case WMeshDescriptor::POINT_LIST:     return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
			default:                              return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		}
	}

	WMesh::WMesh(const WMeshDescriptor& rDescriptor)
		: m_vertexCount{ rDescriptor.vertexCount }
		, m_IndexCount{ rDescriptor.indexCount }
	{
		WAVEE_ASSERT_MESSAGE(rDescriptor.pVertexData, "No vertex data for mesh!");

		m_topology = GetVulkanTopology(rDescriptor.topology);

		m_isIndexed = rDescriptor.pIndexData;

		WaveEDevice pDevice = WaveManager::Instance()->GetDevice();

		// Create vertex buffer
		{
			WBufferDescriptor vertexBufferDescriptor = {};
			vertexBufferDescriptor.isDynamic = false;
			vertexBufferDescriptor.sizeBytes = rDescriptor.vertexCount * rDescriptor.vertexStrideBytes;
			vertexBufferDescriptor.type = WBufferDescriptor::Vertex;
			vertexBufferDescriptor.pInitalData = rDescriptor.pVertexData;

			m_vertexBufferID = WResourceManager::Instance()->CreateResource(vertexBufferDescriptor);
		}

		// Create index buffer if needed
		if (m_isIndexed)
		{
			WBufferDescriptor indexBufferDescriptor = {};
			indexBufferDescriptor.isDynamic = false;
			indexBufferDescriptor.sizeBytes = rDescriptor.indexCount * sizeof(UINT);
			indexBufferDescriptor.type = WBufferDescriptor::Index;
			indexBufferDescriptor.pInitalData = rDescriptor.pIndexData;

			m_indexBufferID = WResourceManager::Instance()->CreateResource(indexBufferDescriptor);
		}
	}

	WMesh::~WMesh()
	{

	}

	void WMesh::Bind() const
	{
		WaveECommandBuffer pCommandBuffer = WaveManager::Instance()->GetCommandBuffer();

		VkDeviceSize offset = 0;
		VkBuffer pBuffer = m_vertexBufferID.GetResource()->GetBuffer();
		vkCmdBindVertexBuffers(pCommandBuffer, 0, 1, &pBuffer, &offset);

		if (m_isIndexed)
		{
			vkCmdBindIndexBuffer(pCommandBuffer, m_indexBufferID.GetResource()->GetBuffer(), 0, VK_INDEX_TYPE_UINT32);
		}
	}
}