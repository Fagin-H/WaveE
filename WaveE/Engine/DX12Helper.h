#pragma once

namespace WaveE 
{
	D3D12_HEAP_PROPERTIES CreateHeapProperties(D3D12_HEAP_TYPE type);
	D3D12_RESOURCE_DESC CreateBufferResourceDesc(UINT width);
	D3D12_RESOURCE_DESC CreateTextureResourceDesc(DXGI_FORMAT format, UINT width, UINT height);

	D3D12_RESOURCE_BARRIER CreateTransitionBarrier(ID3D12Resource* pResource, D3D12_RESOURCE_STATES stateBefore, D3D12_RESOURCE_STATES stateAfter);

	D3D12_BLEND_DESC DefaultBlendDesc();
	D3D12_RASTERIZER_DESC DefaultRasterizerDesc();
	D3D12_DEPTH_STENCIL_DESC DefaultDeptStencilDesc();
}