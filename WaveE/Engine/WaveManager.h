#pragma once

namespace WaveE
{
	// Manager class for WaveE
	class WaveManager
	{
		WAVEE_SINGLETON(WaveManager)

	public:
		// Getter functions
		ID3D12Device1* GetDevice() const { return m_pDevice.Get(); }

	private:
		// DX12 variables
		ComPtr<ID3D12Device1> m_pDevice;
	};
}

