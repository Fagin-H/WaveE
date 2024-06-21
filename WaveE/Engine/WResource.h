#pragma once

namespace WaveE
{
	template<typename Resource>
	struct ResourceID
	{
		UINT id{ UINT_MAX };

		Resource* GetResource() const
		{
			return WResourceManager::Instance()->GetResource(*this);
		}
	};
}