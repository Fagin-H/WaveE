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

	template<typename Resource>
	struct ResourceBlock
	{
		UINT startID{ UINT_MAX };
		UINT numElements{ 0 };
		WDescriptorHeapManager::Allocation allocation{};

		ResourceID<Resource> GetResorce(UINT index)
		{
			WAVEE_ASSERT_MESSAGE(index < numElements, "Resource index out of range!");
			return ResourceID<Resource>{startID + index};
		}

		bool IsValid() const
		{
			return startID != UINT_MAX;
		}
	};
}