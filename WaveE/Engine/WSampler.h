#pragma once

namespace WaveE
{
	struct WSamplerDescriptor
	{
		enum Filter
		{
			Point,
			Linear,
			Anisotropic
		};

		enum AddressMode
		{
			Wrap,
			Clamp,
			Border
		};

		Filter filter{ Linear };
		AddressMode addressMode{ Wrap };
		float mipLODBias{ 0.0f };
		float maxAnisotropy{ 1.0 };
		float minLOD{ 0.0f };
		float maxLOD{ 10 };
		int descriptorSlot{ -1 };
	};

	class WSampler
	{
	public:
		WSampler(const WSamplerDescriptor& rDescriptor);
		~WSampler();

		VkSampler GetSampler() const { return m_pSampler; };
		int GetSlot() const { return m_slot; }

	private:
		VkSampler m_pSampler;
		int m_slot;
	};
}
