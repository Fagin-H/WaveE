#include "stdafx.h"
#include "WMaterialManager.h"

namespace WaveE
{
	WAVEE_SINGLETON_CPP(WMaterialManager)

	void WMaterialManager::MaterialChanged(WMaterial* pMaterial)
	{
		m_needsUpdating = true;

		int index = pMaterial->GetMaterialID();
		WAVEE_ASSERT_MESSAGE(index < MAX_MATERIALS, "Material index out of range!");

		m_highestMaterialID = std::max<int>(m_highestMaterialID, index);

		pMaterial->FillMaterialBuffer(m_vBufferData[index]);
	}

	WMaterialManager::WMaterialManager()
	{
		WBufferDescriptor bufferDesc{};
		bufferDesc.isDynamic = false;
		bufferDesc.type = WBufferDescriptor::Type::StorageRO;
		bufferDesc.sizeBytes = sizeof(MaterialBuffer) * MAX_MATERIALS;
		bufferDesc.descriptorSlot = m_materialBufferReservedSlot;

		m_materialBuffer = WResourceManager::Instance()->CreateResource(bufferDesc);
	}

	void WMaterialManager::EndFrame()
	{
		if (!m_needsUpdating)
		{
			return;
		}

		m_materialBuffer.GetResource()->UploadData(m_vBufferData, sizeof(MaterialBuffer) * (m_highestMaterialID + 1));

		m_needsUpdating = false;
	}
}