#pragma once

namespace WaveE
{
	struct WTextureDescriptor
	{
		enum Format
		{
			RGBA,
			SRGBA,
			RGBAF16,
			DepthFloat
		};

		enum Usage : UINT
		{
			ShaderResource = 1,
			RenderTarget = 1 << 1,
			ResourceAndTarget = ShaderResource | RenderTarget,
		};

		Format format{ RGBA };
		Usage usage{ ShaderResource };
		bool startAsShaderResource{ true };
		UINT width;
		UINT height;
		const void* pInitalData{ nullptr };
		int descriptorSlot{ -1 };
	};

	enum class WImageState
	{
		Undefined,
		TransferDst,
		TransferSrc,
		ShaderRead,
		RenderTarget
	};

	VkAccessFlags GetImageAccessMask(WImageState state, WTextureDescriptor::Format format);
	VkPipelineStageFlags GetImagePipelineStage(WImageState state, WTextureDescriptor::Format format);
	VkImageLayout GetImageLayout(WImageState state, WTextureDescriptor::Format format);

	class WTexture
	{
	public:
		WTexture(const WTextureDescriptor& rDescriptor);
		~WTexture();

		VkImage GetTexture() const { return m_pImage; }

		VkImageView GetView() const { return m_pView; }

		void UploadData(const void* pData);
		
		// Returns true if the state has changed, false otherwise
		bool SetState(WImageState state);

		int GetSlot() const { return m_slot; }

		UINT GetWidth() const { return m_width; }
		UINT GetHeight() const { return m_height; }
		WImageState GetCurrentState() const { return m_currentState; }

		bool IsDepthType() const { return m_isDepthType; }

	private:

		size_t m_sizeBytes{ 0 };
		UINT m_bytesPerPixel{ 0 };
		UINT m_width;
		UINT m_height;
		bool m_isDepthType;
		int m_slot{ -1 };
		WTextureDescriptor::Format m_format;
		WTextureDescriptor::Usage m_usage;
		WImageState m_currentState;
		VkImage m_pImage{ nullptr };
		VkDeviceMemory m_pMemory{ nullptr };
		VkImageView m_pView{ nullptr };

		void UploadData(const void* pData, WImageState currentState, WImageState finalState);
	};
}

