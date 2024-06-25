#pragma once

namespace WaveE
{
	class WTextureLoader
	{
		WAVEE_SINGLETON(WTextureLoader)
	public:
		void LoadPNGAndGetPixels(const wchar_t* filePath, std::vector<glm::vec4> vPixelData, UINT& width, UINT& height);

	private:
		WTextureLoader();
		~WTextureLoader();
	};
}

