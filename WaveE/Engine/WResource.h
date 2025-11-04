#pragma once
#include "WDescriptorHeapManager.h"

namespace WaveE
{
	template<typename Resource>
	struct ResourceID;

	template<typename Resource>
	Resource* GetResourceFromManager(ResourceID<Resource> id);

	template<typename Resource>
	struct ResourceID
	{
		UINT id{ UINT_MAX };

		Resource* GetResource() const
		{
			return GetResourceFromManager(*this);
		}

		bool IsValid() const
		{
			return id != UINT_MAX;
		}
	};
}