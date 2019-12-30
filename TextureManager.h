#pragma once

#include "Basics.h"
#include "Texture.h"

#include <vector>

namespace MelonRenderer
{
	class TextureManager
	{

		std::vector<Texture> m_textures;
		std::vector<VkDescriptorImageInfo> m_textureInfos;

		VkSampler m_textureSampler;
		VkCommandPool m_singleUseBufferCommandPool;

	public:
		bool Init();

		bool CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
		bool CreateTextureImage(VkImage& texture, VkDeviceMemory& textureMemory, const char* filePath);
		bool CreateTextureView(VkImageView& imageView, VkImage image);
		bool CreateTexture(const char* filePath);
		bool CreateTextureSampler();

		bool TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout previousLayout, VkImageLayout desiredLayout);
		bool CopyStagingBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
		bool CreateSingleUseCommand(VkCommandBuffer& commandBuffer);
		bool EndSingleUseCommand(VkCommandBuffer& commandBuffer);

		bool FindMemoryTypeFromProperties(uint32_t typeBits, VkFlags requirements_mask, uint32_t* typeIndex);

		uint32_t GetNumberTextures();
		VkDescriptorImageInfo* GetDescriptorImageInfo();
	};
}