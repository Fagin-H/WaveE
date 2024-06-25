#include "stdafx.h"
#include "WTextureLoader.h"
#include <wincodec.h>

namespace WaveE
{
	WAVEE_SINGLETON_CPP(WTextureLoader)

	WTextureLoader::WTextureLoader()
	{
	}

	void WTextureLoader::LoadPNGAndGetPixels(const wchar_t* filename, std::vector<glm::vec4> vPixelData, UINT& width, UINT& height)
	{
        // Initialize COM for WIC
        HRESULT hr = CoInitialize(nullptr);
        WAVEE_ASSERT_MESSAGE(SUCCEEDED(hr), "Failed to initialize com!");

        // Create WIC factory
        IWICImagingFactory* pFactory = nullptr;
        CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(m_pFactory.GetAddressOf()));

        // Create decoder for the PNG file
        IWICBitmapDecoder* pDecoder = nullptr;
        pFactory->CreateDecoderFromFilename(filename, nullptr, GENERIC_READ, WICDecodeMetadataCacheOnLoad, &pDecoder);

        // Get frame from decoder (assuming single frame PNG)
        IWICBitmapFrameDecode* pFrame = nullptr;
        pDecoder->GetFrame(0, &pFrame);

        // Get size of the image
        pFrame->GetSize(&width, &height);

        // Calculate total pixel count
        UINT totalPixels = width * height;

        // Prepare vector to hold pixel data
        vPixelData.reserve(totalPixels);

        // Convert pixel data to format
        WICPixelFormatGUID format;
        pFrame->GetPixelFormat(&format);

        if (IsEqualGUID(format, GUID_WICPixelFormat32bppRGBA))
        {
            // Directly copy pixels if format is 32bpp BGRA
            pFrame->CopyPixels(nullptr, width * sizeof(glm::vec4), totalPixels * sizeof(glm::vec4), reinterpret_cast<BYTE*>(vPixelData.data()));
        }
        else
        {
            // Convert to 32bpp RGBA format if not already in that format
            IWICFormatConverter* pConverter = nullptr;
            pFactory->CreateFormatConverter(&pConverter);
            pConverter->Initialize(pFrame, GUID_WICPixelFormat32bppRGBA, WICBitmapDitherTypeNone, nullptr, 0.0, WICBitmapPaletteTypeCustom);

            // Copy converted pixels
            pConverter->CopyPixels(nullptr, width * sizeof(glm::vec4), totalPixels * sizeof(glm::vec4), reinterpret_cast<BYTE*>(vPixelData.data()));

            // Release the converter
            pConverter->Release();
        }

        // Release resources
        pFrame->Release();
        pDecoder->Release();
        pFactory->Release();

        // Uninitialize COM
        CoUninitialize();
	}

	WTextureLoader::~WTextureLoader()
	{
		
	}
}