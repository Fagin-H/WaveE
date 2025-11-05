#pragma once
#include "WaveManager.h"

namespace WaveE
{
	constexpr int MAX_MATERIALS{ 256 };
	class WMaterialManager
	{
		WAVEE_SINGLETON(WMaterialManager);
	public:

		int GetMaterialSlot() const{ return m_materialBufferReservedSlot; }

		void MaterialChanged(WMaterial* pMaterial);
	private:
		bool m_needsUpdating{ true };
		int m_materialBufferReservedSlot{ 0 };
		int m_highestMaterialID{ 0 };
		MaterialBuffer m_vBufferData[MAX_MATERIALS];
		ResourceID<WBuffer> m_materialBuffer{};
		WMaterialManager();

		friend class WaveManager;
		void EndFrame();
	};
}

