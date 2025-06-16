#pragma once
#include "WMesh.h"
#include "WResource.h"
#include "WBuffer.h"

namespace WaveE
{
	class WBottomLevelAS;
	class WTopLevelAS;

	struct WBLASDescriptor
	{
		enum Flags : UINT
		{
			NONE = 0,
			IS_OPAQUE = 1,
			ALLOW_UPDATE = 1 << 1,
		};

		ResourceID<WMesh> meshID;
		Flags flags{ NONE };
		ResourceID<WBottomLevelAS> previousResult{};
	};

	class WBottomLevelAS
	{
	public:
		WBottomLevelAS(const WBLASDescriptor& rDescriptor);
		~WBottomLevelAS();

		ResourceID<WBuffer> GetASBuffer() const { return m_ASBuffer; }
	private:
		static ResourceID<WBuffer> m_scratchBuffer;
		ResourceID<WBuffer> m_ASBuffer{};

	};

	struct WTLASInstanceDescriptor
	{
		ResourceID<WBottomLevelAS> BLAS;
		wma::mat4 transform;
		UINT instanceID;
		UINT hitGroup{ 0 };
	};

	struct WTLASDescriptor
	{
		enum Flags : UINT
		{
			NONE = 0,
			ALLOW_UPDATE = 1,
		};

		WTLASInstanceDescriptor* pTLASInstanceDescriptors;
		UINT numTLASInstanceDescriptors;
		Flags flags{ NONE };
		ResourceID<WTopLevelAS> previousResult{};
	};

	class WTopLevelAS
	{
	public:
		WTopLevelAS(const WTLASDescriptor& rDescriptor);
		~WTopLevelAS();

		ResourceID<WBuffer> GetASBuffer() const { return m_ASBuffer; }
	private:
		static ResourceID<WBuffer> m_scratchBuffer;
		ResourceID<WBuffer> m_descriptorBuffer;
		ResourceID<WBuffer> m_ASBuffer;
	};
}