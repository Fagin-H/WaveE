#pragma once
#include "WResource.h"
#include "WPipeline.h"
#include "WBuffer.h"

namespace WaveE
{
	struct WShaderBindingTableDescriptor
	{
		ResourceID<WPipelineRT> pipeline{};
		std::wstring rayGenExport;
		std::vector<std::wstring> missExports;
		std::vector<std::wstring> hitGroupExports;

		UINT rayGenRecordSize = 0;
		UINT missRecordSize = 0;
		UINT hitGroupRecordSize = 0;

		std::vector<BYTE> rayGenLocalRootArgs;
		std::vector<std::vector<BYTE>> missLocalRootArgs;
		std::vector<std::vector<BYTE>> hitGroupLocalRootArgs;
	};

	class WShaderBindingTable
	{
	public:
		WShaderBindingTable(const WShaderBindingTableDescriptor& rDescriptor);
		~WShaderBindingTable();
		
		ResourceID<WBuffer> GetBuffer() const { return m_shaderBindingTableBufferID; }

		UINT GetRayGenRecordSize() const { return m_rayGenRecordSize; }
		UINT GetMissRecordSize() const { return m_missRecordSize; }
		UINT GetHitGroupRecordSize() const { return m_hitGroupRecordSize; }
		UINT GetMissTableSize() const { return m_missTableSize; }
		UINT GetHitTableSize() const { return m_hitTableSize; }
		UINT GetRayGenStartOffset() const { return m_rayGenStartOffset; }
		UINT GetMissStartOffset() const { return m_missStartOffset; }
		UINT GetHitStartOffset() const { return m_hitStartOffset; }

	private:
		UINT GetMaxRecorldSize(const std::vector<std::vector<BYTE>>& args);

		ResourceID<WBuffer> m_shaderBindingTableBufferID;

		UINT m_rayGenRecordSize{ 0 };
		UINT m_missRecordSize{ 0 };
		UINT m_hitGroupRecordSize{ 0 };
		UINT m_missTableSize{ 0 };
		UINT m_hitTableSize{ 0 };

		UINT m_rayGenStartOffset{ 0 };
		UINT m_missStartOffset{ 0 };
		UINT m_hitStartOffset{ 0 };

		UINT m_missCount{ 0 };
		UINT m_hitGroupCount{ 0 };
	};
}