#include "stdafx.h"
#include "WTextureLoader.h"
#include <wincodec.h>

namespace WaveE
{
	WAVEE_SINGLETON_CPP(WTextureLoader)

	WTextureLoader::WTextureLoader()
	{
		// Initialize COM for WIC
		HRESULT hr = CoInitialize(nullptr);
		WAVEE_ASSERT_MESSAGE(SUCCEEDED(hr), "Failed to initialize com!");
	}

	void WTextureLoader::LoadPNGAndGetPixels(const wchar_t* filePath, std::vector<glm::vec4> vPixelData, UINT& width, UINT& height)
	{
        // Create WIC factory
        IWICImagingFactory* pFactory = nullptr;
        HRESULT hr = CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pFactory));
        WAVEE_ASSERT_MESSAGE(SUCCEEDED(hr), "Failed to create factory!");

        // Create decoder for the PNG file
        IWICBitmapDecoder* pDecoder = nullptr;
        hr = pFactory->CreateDecoderFromFilename(filePath, nullptr, GENERIC_READ, WICDecodeMetadataCacheOnLoad, &pDecoder);
        WAVEE_ASSERT_MESSAGE(SUCCEEDED(hr), "Failed to create decoder!");

        // Get frame from decoder (assuming single frame PNG)
        IWICBitmapFrameDecode* pFrame = nullptr;
        hr = pDecoder->GetFrame(0, &pFrame);
        WAVEE_ASSERT_MESSAGE(SUCCEEDED(hr), "Failed to get frame!");

        // Get size of the image
        hr = pFrame->GetSize(&width, &height);
        WAVEE_ASSERT_MESSAGE(SUCCEEDED(hr), "Failed to get size!");

        // Calculate total pixel count
        UINT totalPixels = width * height;

        // Prepare vector to hold pixel data
        vPixelData.reserve(totalPixels);

        // Convert pixel data to format
        WICPixelFormatGUID format;
        hr = pFrame->GetPixelFormat(&format);
        WAVEE_ASSERT_MESSAGE(SUCCEEDED(hr), "Failed toget pixel format!");

        if (IsEqualGUID(format, GUID_WICPixelFormat32bppRGBA))
        {
            // Directly copy pixels if format is 32bpp BGRA
            hr = pFrame->CopyPixels(nullptr, width * sizeof(glm::vec4), totalPixels * sizeof(glm::vec4), reinterpret_cast<BYTE*>(vPixelData.data()));
            WAVEE_ASSERT_MESSAGE(SUCCEEDED(hr), "Failed to copy pixels!");
        }
        else
        {
            // Convert to 32bpp RGBA format if not already in that format
            IWICFormatConverter* pConverter = nullptr;
            hr = pFactory->CreateFormatConverter(&pConverter);
            WAVEE_ASSERT_MESSAGE(SUCCEEDED(hr), "Failed to create converter!");
            hr = pConverter->Initialize(pFrame, GUID_WICPixelFormat32bppRGBA, WICBitmapDitherTypeNone, nullptr, 0.0, WICBitmapPaletteTypeCustom);
            WAVEE_ASSERT_MESSAGE(SUCCEEDED(hr), "Failed to initialize converter!");

            // Copy converted pixels
            hr = pConverter->CopyPixels(nullptr, width * sizeof(glm::vec4), totalPixels * sizeof(glm::vec4), reinterpret_cast<BYTE*>(vPixelData.data()));
            WAVEE_ASSERT_MESSAGE(SUCCEEDED(hr), "Failed to copy pixels!");
            // Release the converter
            pConverter->Release();
        }

        // Release resources
        pFrame->Release();
        pDecoder->Release();
        pFactory->Release();
	}

	WTextureLoader::~WTextureLoader()
	{
		// Uninitialize COM
		CoUninitialize();
	}
}