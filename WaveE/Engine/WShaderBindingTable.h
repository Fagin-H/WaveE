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
		WShaderBindingTable(const WShaderBindingTableDescriptor& rDescriptor);
		~WShaderBindingTable();

	private:
		UINT GetMaxRecorldSize(const std::vector<std::vector<BYTE>>& args);

		ResourceID<WBuffer> m_shaderBindingTableBufferID;

		UINT m_rayGenRecordSize{ 0 };
		UINT m_missRecordSize{ 0 };
		UINT m_hitGroupRecordSize{ 0 };

		UINT m_missCount{ 0 };
		UINT m_hitGroupCount{ 0 };
	};
}