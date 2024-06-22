#pragma once
#include "WDescriptorHeapManager.h"
#include <functional>
#include "WUploadManager.h"
#include "WRootSigniture.h"
#include "WSampler.h"
#include "WResourceManager.h"


namespace WaveE
{
	struct WaveEDescriptor
	{
		UINT width{ 1280 };
		UINT height{ 720 };
		const char* title{ "WaveE" };
	};

	// Manager class for WaveE
	class WaveManager
	{
		// Singleton setup
		WaveManager(const WaveManager&) = delete; 
		WaveManager& operator=(const WaveManager&) = delete; 
	public: 
		static WaveManager* Instance() { return ms_pInstance; }

		static void Init(const WaveEDescriptor& rDescriptor); 

		static void Uninit(); 

	private: 
		static WaveManager* ms_pInstance;

	public:
		void BeginFrame();
		void EndFrame();

		// Getter functions
		WaveEDevice* GetDevice() const { return m_pDevice.Get(); }
		WaveECommandList* GetCommandList() const { return m_pCommandList.Get(); }
		WaveECommandQueue* GetCommandQueue() const { return m_pCommandQueue.Get(); }

		WDescriptorHeapManager* GetCBV_SRV_UAVHeap() { return &m_cbvSrvUavHeap; }
		WDescriptorHeapManager* GetRTVHeap() { return &m_rtvHeap; }
		WDescriptorHeapManager* GetDSVHeap() { return &m_dsvHeap; }
		WDescriptorHeapManager* GetSamplerHeap() { return &m_samplerHeap; }

		WUploadManager* GetUploadManager() { return &m_uploadManager; }

		WRootSigniture* GetDefaultRootSigniture() { return &m_defaultRootSigniture; }

		D3D12_INPUT_LAYOUT_DESC GetDefaultInputLayout() const { return m_defaultInputLayout; }

		UINT GetWidth() const { return m_width; }
		UINT GetHeight() const { return m_height; }

		enum SamplerType
		{
			WRAP_POINT,
			WRAP_LINEAR,
			CLAMP_POINT,
			CLAMP_LINEAR
		};
		ResourceID<WSampler> GetDefaultSampler(SamplerType type = WRAP_LINEAR);

		// Drawing functions
		void SetPipelineState(ResourceID<WPipeline> id);

		enum SlotIndex : UINT
		{
			DEFAULT_SAMPLERS = 0,
			GLOBAL_CBV_SRV,
			GLOBAL_SAMPLERS,
			MATERIAL_CBV_SRV,
			MATERIAL_SAMPLERS
		};
		void BindBuffer(ResourceID<WBuffer> id, SlotIndex slot);
		void BindTexture(ResourceID<WTexture> id, SlotIndex slot);
		void BindSampler(ResourceID<WSampler> id, SlotIndex slot);
		void BindBuffers(ResourceBlock<WBuffer> rb, SlotIndex slot);
		void BindTextures(ResourceBlock<WTexture> rb, SlotIndex slot);
		void BindSamplers(ResourceBlock<WSampler> rb, SlotIndex slot);
		void BindResource(WDescriptorHeapManager::Allocation allocation, SlotIndex slot);

		// Sets the render target, viewport, and scissor rect for drawing to the whole screen
		void SetRenderTarget(ResourceID<WTexture> RTVId, ResourceID<WTexture> DSVId = {});
		void ClearRenderTarget(ResourceID<WTexture> id, glm::vec4 colour);
		void ClearDepthStencilTarget(ResourceID<WTexture> id, float depth = 1, UINT stencil = 0);

		void DrawMeshWithCurrentParamaters(ResourceID<WMesh> id, UINT count = 1);
		void DrawIndexedMeshWithCurrentParamaters(ResourceID<WMesh> id, UINT count = 1);


		void SetDefaultRootSigniture() { SetRootSigniture(&m_defaultRootSigniture); }
		void SetRootSigniture(WRootSigniture* pRootSigniture);

	private:
		WaveManager(const WaveEDescriptor& rDescriptor);
		~WaveManager();

		void InitWindow(const WaveEDescriptor& rDescriptor);
		void InitDX12(const WaveEDescriptor& rDescriptor);

		static LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
		void GetHardwareAdapter(
			IDXGIFactory1* pFactory,
			IDXGIAdapter1** ppAdapter,
			bool requestHighPerformanceAdapter = false);

		void CreateDefaultRootSigniture();

		// Create the hlsl include file for named slots
		// If the default slot layout changes the shaders don't need to be changes
		// Just replace the old hlsli file with the new one and recompile
		void CreateSlotHLSLIFile();

		std::string GetShaderDirectory();

		bool IsSamplerSlot(SlotIndex index);
		bool IsCBV_SRV_UAVSlot(SlotIndex index);

		// DX12 variables
		static const UINT m_frameCount{ 2 };

		ComPtr<WaveEDevice> m_pDevice;
		ComPtr<WaveECommandList> m_pCommandList;
		ComPtr<WaveECommandQueue> m_pCommandQueue;
		ComPtr<IDXGISwapChain3> m_pSwapChain;
		ComPtr<ID3D12CommandAllocator> m_pCommandAllocators[m_frameCount];

		UINT m_frameIndex;
		HANDLE m_fenceEvent;
		ComPtr<ID3D12Fence> m_pFence;
		UINT64 m_fenceValues[m_frameCount];

		// Windows variables
		HWND m_hwnd{ NULL };
		HINSTANCE m_hInstance{ GetModuleHandle(NULL) };

		// WaveE variables
		WDescriptorHeapManager m_cbvSrvUavHeap;
		WDescriptorHeapManager m_rtvHeap;
		WDescriptorHeapManager m_dsvHeap;
		WDescriptorHeapManager m_samplerHeap;

		WUploadManager m_uploadManager;

		WRootSigniture m_defaultRootSigniture;

		ResourceBlock<WSampler> m_defaultSamplers;

		D3D12_INPUT_LAYOUT_DESC m_defaultInputLayout;

		UINT m_width;
		UINT m_height;
		bool m_hasWindowSizeChanged{ false };


		const UINT m_descriptorHeapCountCBV_SRV_UAV{ 1024 };
		const UINT m_descriptorHeapCountRTV{ 16 };
		const UINT m_descriptorHeapCountDSV{ 16 };
		const UINT m_descriptorHeapCountSampler{ 16 };

		const UINT m_uploadBufferCount{ 10 };
		const UINT m_uploadBufferSize{ 1024 * 1024 * 32 };

		const UINT m_defaultSamplerCount{ 4 };
		const UINT m_globalCBVCount{ 1 };
		const UINT m_globalSRVCount{ 4 };
		const UINT m_globalSamplerCount{ m_globalSRVCount };
		const UINT m_materialCBVCount{ 1 };
		const UINT m_materialSRVCount{ 4 };
		const UINT m_materialSamplerCount{ m_materialSRVCount };
	};
}

