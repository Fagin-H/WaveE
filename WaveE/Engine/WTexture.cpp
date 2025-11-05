#include "stdafx.h"
#include "WTexture.h"
#include "WaveManager.h"

namespace WaveE
{
	size_t GetBytesPerPixel(DXGI_FORMAT format)
	{
		switch (format)
		{
		case DXGI_FORMAT_R8G8B8A8_UNORM: [[fallthrough]];
		case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB: [[fallthrough]];
		case DXGI_FORMAT_D32_FLOAT: [[fallthrough]];
		case DXGI_FORMAT_R32_TYPELESS: return 4;
		case DXGI_FORMAT_R16G16B16A16_FLOAT: return 8;
		default:
			WAVEE_ASSERT_MESSAGE(false, "Format not supported!");
		}

		return -1;
	}

	VkFormat GetVulkanFormat(WTextureDescriptor::Format format) 
	{
		switch (format)
		{
			case WTextureDescriptor::Format::RGBA:
				return VK_FORMAT_R8G8B8A8_UNORM;
			case WTextureDescriptor::Format::SRGBA:
				return VK_FORMAT_R8G8B8A8_SRGB;
			case WTextureDescriptor::Format::RGBAF16:
				return VK_FORMAT_R16G16B16A16_SFLOAT;
			case WTextureDescriptor::Format::DepthFloat:
				return VK_FORMAT_D32_SFLOAT;
			default:
				WAVEE_ASSERT_MESSAGE(false, "Unsupported texture format!");
				return VK_FORMAT_UNDEFINED;
		}
	}

	VkImageUsageFlags GetImageUsageFlags(WTextureDescriptor::Usage usage)
	{
		VkImageUsageFlags flags = 0;

		if (usage & WTextureDescriptor::ShaderResource)
		{
			// TransferSrc allows copying the image back if needed
			flags |= VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
		}

		if (usage & WTextureDescriptor::RenderTarget)
		{
			// TransferDst allows uploading data to the render target
			flags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		}

		return flags;
	}

	UINT GetBytesPerPixel(WTextureDescriptor::Format format)
	{
		switch (format)
		{
			case WTextureDescriptor::Format::RGBA:
			case WTextureDescriptor::Format::SRGBA:
				return 4; // 4 channels × 1 byte each
			case WTextureDescriptor::Format::RGBAF16:
				return 8; // 4 channels × 2 bytes each (16-bit float)
			case WTextureDescriptor::Format::DepthFloat:
				return 4; // 32-bit float depth
			default:
				WAVEE_ASSERT_MESSAGE(false, "Unsupported texture format for bytes per pixel!");
				return 0;
		}
	}

	WTexture::WTexture(const WTextureDescriptor& rDescriptor)
	{
		m_width = rDescriptor.width;
		m_height = rDescriptor.height;
		m_currentState = rDescriptor.startAsShaderResource ? WImageState::ShaderRead : WImageState::RenderTarget;
		m_format = rDescriptor.format;
		m_usage = rDescriptor.usage;

		WaveEDevice pDevice = WaveManager::Instance()->GetDevice();

		// Create VkImage
		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.format = GetVulkanFormat(rDescriptor.format);
		imageInfo.extent = { rDescriptor.width, rDescriptor.height, 1 };
		imageInfo.mipLevels = 1;
		imageInfo.arrayLayers = 1;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageInfo.usage = GetImageUsageFlags(rDescriptor.usage);
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

		VkResult result = vkCreateImage(pDevice, &imageInfo, nullptr, &m_pImage);
		WAVEE_ASSERT_MESSAGE(result == VK_SUCCESS, "Failed to create image!");

		// Allocate and bind memory
		VkMemoryRequirements memReq;
		vkGetImageMemoryRequirements(pDevice, m_pImage, &memReq);

		uint32_t memoryTypeIndex = FindMemoryType(
			memReq.memoryTypeBits,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
		);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memReq.size;
		allocInfo.memoryTypeIndex = memoryTypeIndex;

		result = vkAllocateMemory(pDevice, &allocInfo, nullptr, &m_pMemory);
		WAVEE_ASSERT_MESSAGE(result == VK_SUCCESS, "Failed to allocate memory for image!");

		result = vkBindImageMemory(pDevice, m_pImage, m_pMemory, 0);
		WAVEE_ASSERT_MESSAGE(result == VK_SUCCESS, "Failed to bind image memory!");

		// Allocate bindless descriptor slot
		if (rDescriptor.usage & WTextureDescriptor::ShaderResource)
		{
			m_slot = WaveManager::Instance()->GetDescriptorManager()->AddResource(m_pView, rDescriptor.descriptorSlot);
		}

		// Upload initial data if present
		if (rDescriptor.pInitalData)
		{
			WaveManager::Instance()->GetUploadManager()->UploadDataToTexture(
				m_pImage,
				rDescriptor.pInitalData,
				rDescriptor.width,
				rDescriptor.height,
				GetBytesPerPixel(rDescriptor.format),
				m_format,
				WImageState::Undefined,
				m_currentState
			);
		}

		// Create default VkImageView for shader access
		VkImageViewCreateInfo viewInfo{};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = m_pImage;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = GetVulkanFormat(rDescriptor.format);
		viewInfo.subresourceRange.aspectMask = (rDescriptor.format == WTextureDescriptor::DepthFloat) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;

		result = vkCreateImageView(pDevice, &viewInfo, nullptr, &m_pView);
		WAVEE_ASSERT_MESSAGE(result == VK_SUCCESS, "Failed to create image view!");
	}

	WTexture::~WTexture()
	{
	}

	void WTexture::UploadData(const void* pData)
	{
		UploadData(pData, m_currentState, m_currentState);
	}

	void WTexture::UploadData(const void* pData, WImageState currentState, WImageState finalState)
	{
		WaveManager::Instance()->GetUploadManager()->UploadDataToTexture(m_pImage, pData, m_width, m_height, m_bytesPerPixel, m_format, currentState, finalState);
	}

	bool WTexture::SetState(WImageState state)
	{
		if (m_currentState == state)
			return false;

		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = GetImageLayout(m_currentState, m_format);
		barrier.newLayout = GetImageLayout(state, m_format);
		barrier.srcAccessMask = GetImageAccessMask(m_currentState, m_format);
		barrier.dstAccessMask = GetImageAccessMask(state, m_format);
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = m_pImage;
		barrier.subresourceRange.aspectMask =
			(m_format == WTextureDescriptor::DepthFloat) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;

		WaveECommandBuffer pCommandBuffer = WaveManager::Instance()->GetCommandBuffer();
		vkCmdPipelineBarrier(
			pCommandBuffer,
			GetImagePipelineStage(m_currentState, m_format),
			GetImagePipelineStage(state, m_format),
			0,
			0, nullptr,
			0, nullptr,
			1, &barrier
		);

		m_currentState = state;

		return true;
	}

	VkAccessFlags GetImageAccessMask(WImageState state, WTextureDescriptor::Format format)
	{
		switch (state)
		{
			case WImageState::Undefined:
				return 0;
			case WImageState::TransferDst:
				return VK_ACCESS_TRANSFER_WRITE_BIT;
			case WImageState::TransferSrc:
				return VK_ACCESS_TRANSFER_READ_BIT;
			case WImageState::ShaderRead:
				return VK_ACCESS_SHADER_READ_BIT;
			case WImageState::RenderTarget:
				return (format == WTextureDescriptor::DepthFloat)
					? VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT
					: VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			default:
				return 0;
		}
	}

	VkPipelineStageFlags GetImagePipelineStage(WImageState state, WTextureDescriptor::Format format)
	{
		switch (state)
		{
			case WImageState::Undefined:
				return VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			case WImageState::TransferDst:
			case WImageState::TransferSrc:
				return VK_PIPELINE_STAGE_TRANSFER_BIT;
			case WImageState::ShaderRead:
				return VK_PIPELINE_STAGE_VERTEX_SHADER_BIT |
					VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT |
					VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
			case WImageState::RenderTarget:
				if (format == WTextureDescriptor::DepthFloat)
				{
					return VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
						VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
				}
				else
				{
					return VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
				}
			default:
				return VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		}
	}

	VkImageLayout GetImageLayout(WImageState state, WTextureDescriptor::Format format)
	{
		switch (state)
		{
			case WImageState::Undefined:
				return VK_IMAGE_LAYOUT_UNDEFINED;
			case WImageState::TransferDst:
				return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			case WImageState::TransferSrc:
				return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			case WImageState::ShaderRead:
				return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			case WImageState::RenderTarget:
				if (format == WTextureDescriptor::DepthFloat)
				{
					return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
				}
				else
				{
					return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
				}
			default:
				return VK_IMAGE_LAYOUT_UNDEFINED;
		}
	}
}