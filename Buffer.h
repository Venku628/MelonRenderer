#pragma once

#include "Basics.h"

namespace MelonRenderer
{
	//from texture
	bool CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
	bool CreateTextureImage(VkImage& texture, VkDeviceMemory& textureMemory, const char* filePath);
	bool CopyStagingBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
	bool CreateSingleUseCommand(VkCommandBuffer& commandBuffer);
	bool EndSingleUseCommand(VkCommandBuffer& commandBuffer);


	//from renderer
	bool CopyStagingBufferToBuffer(VkBuffer cpuVisibleBuffer, VkBuffer gpuOnlyBuffer, VkDeviceSize size);
	bool CreateOptimalBuffer(VkBuffer& buffer, VkDeviceMemory& bufferMemory, const void* data, VkDeviceSize bufferSize, VkBufferUsageFlagBits bufferUsage);



	bool FindMemoryTypeFromProperties(uint32_t typeBits, VkFlags requirements_mask, uint32_t* typeIndex);

}
