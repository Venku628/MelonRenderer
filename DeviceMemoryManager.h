#pragma once

#include <vector>

#include "Basics.h"
#include "Texture.h"

namespace MelonRenderer
{

	class DeviceMemoryManager
	{
		std::vector<Texture> m_textures;
		std::vector<VkDescriptorImageInfo> m_textureInfos;

		VkSampler m_textureSampler;
		VkCommandPool m_singleUseBufferCommandPool;

		VkPhysicalDeviceMemoryProperties m_physicalDeviceMemoryProperties;

	public:
		bool Init(VkPhysicalDeviceMemoryProperties& physicalDeviceMemoryProperties);

		bool CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory) const;
		bool CreateOptimalBuffer(VkBuffer& buffer, VkDeviceMemory& bufferMemory, const void* data, VkDeviceSize bufferSize, VkBufferUsageFlagBits bufferUsage) const;

		bool CreateTextureImage(VkImage& texture, VkDeviceMemory& textureMemory, unsigned char* pixelData, int width, int height);
		bool CreateTextureView(VkImageView& imageView, VkImage image);
		bool CreateTexture(const char* filePath);
		bool CreateTextureSampler();
		bool TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout previousLayout, VkImageLayout desiredLayout);
		
		bool CopyStagingBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) const;
		bool CopyStagingBufferToBuffer(VkBuffer cpuVisibleBuffer, VkBuffer gpuOnlyBuffer, VkDeviceSize size) const;

		bool CreateSingleUseCommand(VkCommandBuffer& commandBuffer) const;
		bool EndSingleUseCommand(VkCommandBuffer& commandBuffer) const;

		bool FindMemoryTypeFromProperties(uint32_t typeBits, VkFlags requirements_mask, uint32_t* typeIndex) const;

		uint32_t GetNumberTextures();
		VkDescriptorImageInfo* GetDescriptorImageInfo();

	};

}