#pragma once

namespace WaveE
{
	struct IWICImagingFactory;

	class WTextureLoader
	{
		WAVEE_SINGLETON(WTextureLoader)
	public:
		void LoadPNGAndGetPixels(const wchar_t* filename, std::vector<glm::vec4> vPixelData, UINT& width, UINT& height);

	private:
		WTextureLoader();
		~WTextureLoader();
	};
}

