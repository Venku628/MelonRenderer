#include "TextureManager.h"
#define STB_IMAGE_IMPLEMENTATION 
#include <stb_image.h>

namespace MelonRenderer
{
	bool TextureManager::Init()
	{
		VkCommandPoolCreateInfo cmdPoolInfo = {};
		cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		cmdPoolInfo.pNext = NULL;
		cmdPoolInfo.queueFamilyIndex = 0; //TODO: make variable as soon as it is defined as a non constant
		cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT | VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;

		VkResult result = vkCreateCommandPool(Device::Get().m_device, &cmdPoolInfo, NULL, &m_singleUseBufferCommandPool);
		if (result != VK_SUCCESS)
		{
			Logger::Log("Could not create single use command buffer pool for texture manger.");
			return false;
		}

		CreateTextureSampler();

		return true;
	}
	bool TextureManager::CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
	{
		VkBufferCreateInfo bufferInfo = {};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = size;
		bufferInfo.usage = usage;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VkResult result = vkCreateBuffer(Device::Get().m_device, &bufferInfo, nullptr, &buffer);
		if (result != VK_SUCCESS) {
			Logger::Log("Could not create buffer.");
			return false;
		}

		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(Device::Get().m_device, buffer, &memRequirements);

		VkMemoryAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		FindMemoryTypeFromProperties(memRequirements.memoryTypeBits, properties, &allocInfo.memoryTypeIndex);

		result = vkAllocateMemory(Device::Get().m_device, &allocInfo, nullptr, &bufferMemory);
		if (result != VK_SUCCESS) {
			Logger::Log("Could not allocate buffer memory.");
			return false;
		}

		result = vkBindBufferMemory(Device::Get().m_device, buffer, bufferMemory, 0);
		if (result != VK_SUCCESS)
		{
			Logger::Log("Could not bind buffer to memory.");
			return false;
		}

		return true;
	}

	bool TextureManager::CreateTextureImage(VkImage& texture, VkDeviceMemory& textureMemory, const char* filePath)
	{
		//using int because of compatibility issues
		int width, height, channels;
		stbi_uc* pixelData = stbi_load(filePath, &width, &height, &channels, STBI_rgb_alpha);

		if (pixelData == nullptr)
		{
			Logger::Log("Could not load texture from file path.");
			return false;
		}

		VkDeviceSize imageSize = static_cast<double>(width)* static_cast<double>(height) * 4; //4 for STBI_rgb_alpha

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		if (!CreateBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory))
		{
			Logger::Log("Could not create staging buffer for texture.");
			return false;
		}

		void* data;
		VkResult result = vkMapMemory(Device::Get().m_device, stagingBufferMemory, 0, imageSize, 0, &data);
		if (result != VK_SUCCESS)
		{
			Logger::Log("Could not map memory for staging buffer.");
			return false;
		}
		memcpy(data, pixelData, static_cast<size_t>(imageSize));
		stbi_image_free(pixelData);
		vkUnmapMemory(Device::Get().m_device, stagingBufferMemory);

		//TODO: make subfunction if used elsewhere

		VkImageCreateInfo imageInfo = {};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.pNext = nullptr;
		imageInfo.flags = 0;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
		imageInfo.extent.width = static_cast<uint32_t>(width);
		imageInfo.extent.height = static_cast<uint32_t>(height);
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = 1;
		imageInfo.arrayLayers = 1;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT; //only relevant to images used as attachments
		imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		result = vkCreateImage(Device::Get().m_device, &imageInfo, nullptr, &texture);
		if (result != VK_SUCCESS)
		{
			Logger::Log("Could not create image for texture.");
			return false;
		}

		VkMemoryRequirements memoryRequirements;
		vkGetImageMemoryRequirements(Device::Get().m_device, texture, &memoryRequirements);
		VkMemoryAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.pNext = nullptr;
		allocInfo.allocationSize = memoryRequirements.size;
		if (!FindMemoryTypeFromProperties(memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &allocInfo.memoryTypeIndex))
		{
			Logger::Log("Could find memory type from properties for texture image.");
			return false;
		}

		result = vkAllocateMemory(Device::Get().m_device, &allocInfo, nullptr, &textureMemory);
		if (result != VK_SUCCESS)
		{
			Logger::Log("Could not allocate memory for texture image.");
			return false;
		}

		result = vkBindImageMemory(Device::Get().m_device, texture, textureMemory, 0);
		if (result != VK_SUCCESS)
		{
			Logger::Log("Could not bind memory for texture image.");
			return false;
		}

		if (!TransitionImageLayout(texture, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL))
		{
			Logger::Log("Could not transition image layout before uplodading data to image.");
			return false;
		}

		if (!CopyStagingBufferToImage(stagingBuffer, texture, width, height))
		{
			Logger::Log("Could not transition image layout before uplodading data to image.");
			return false;
		}

		if (!TransitionImageLayout(texture, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL))
		{
			Logger::Log("Could not create single use command buffer for transition of image layout to be read as a texture.");
			return false;
		}

		vkDestroyBuffer(Device::Get().m_device, stagingBuffer, nullptr);
		vkFreeMemory(Device::Get().m_device, stagingBufferMemory, nullptr);

		return true;
	}

	bool TextureManager::CreateTextureView(VkImageView& imageView, VkImage image)
	{
		VkImageViewCreateInfo viewInfo = {};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.pNext = nullptr;
		viewInfo.flags = 0;
		viewInfo.image = image;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
		viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;

		VkResult result = vkCreateImageView(Device::Get().m_device, &viewInfo, nullptr, &imageView);
		if (result != VK_SUCCESS)
		{
			Logger::Log("Could not create image view for texture.");
			return false;
		}

		return true;
	}

	bool TextureManager::CreateTexture(const char* filePath)
	{
		Texture texture;

		if (!CreateTextureImage(texture.m_textureImage, texture.m_textureMemory, filePath))
		{
			Logger::Log("Could not create texture image and memory.");
			return false;
		}

		if (!CreateTextureView(texture.m_textureImageView, texture.m_textureImage))
		{
			Logger::Log("Could not create texture view.");
			return false;
		}

		//TODO: parameter for sampler
		VkDescriptorImageInfo textureInfo = {};
		textureInfo.sampler = m_textureSampler;
		textureInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		textureInfo.imageView = texture.m_textureImageView;
		m_textureInfos.emplace_back(textureInfo);
		m_textures.emplace_back(texture);

		return true;
	}

	bool TextureManager::CreateTextureSampler()
	{
		VkSamplerCreateInfo samplerInfo = {};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.pNext = nullptr;
		samplerInfo.flags = 0;
		samplerInfo.magFilter = VK_FILTER_LINEAR;
		samplerInfo.minFilter = VK_FILTER_LINEAR;
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.mipLodBias = 0.0f;
		samplerInfo.anisotropyEnable = VK_TRUE;
		samplerInfo.maxAnisotropy = 16;
		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
		samplerInfo.minLod = 0.0f;
		samplerInfo.maxLod = 0.0f;
		samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		samplerInfo.unnormalizedCoordinates = VK_FALSE; //TODO: change this to true, so coords are always 0-1

		VkResult result = vkCreateSampler(Device::Get().m_device, &samplerInfo, nullptr, &m_textureSampler);
		if (result != VK_SUCCESS)
		{
			Logger::Log("Could not create texture sampler.");
			return false;
		}

		//TODO: output? evaluate texture strategy first
		//m_descriptorImageInfoTexture.sampler = m_textureSampler;

		return true;
	}

	bool TextureManager::TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout previousLayout, VkImageLayout desiredLayout)
	{
		VkCommandBuffer layoutTransitionCommandBuffer;
		if (!CreateSingleUseCommand(layoutTransitionCommandBuffer))
		{
			Logger::Log("Could not create single use command buffer for transition of image layout.");
			return false;
		}

		VkPipelineStageFlags sourceStageFlags, destinationStageFlags;

		VkImageMemoryBarrier barrier = {};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.pNext = nullptr;
		barrier.oldLayout = previousLayout;
		barrier.newLayout = desiredLayout;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = image;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;

		if (previousLayout == VK_IMAGE_LAYOUT_UNDEFINED && desiredLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
		{
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

			sourceStageFlags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destinationStageFlags = VK_PIPELINE_STAGE_TRANSFER_BIT;
		}
		else if (previousLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && desiredLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
		{
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			sourceStageFlags = VK_PIPELINE_STAGE_TRANSFER_BIT;
			destinationStageFlags = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		}
		else
		{
			Logger::Log("Could find valid configuration for image layout transition.");
			return false;
		}

		vkCmdPipelineBarrier(layoutTransitionCommandBuffer, sourceStageFlags, destinationStageFlags, 0, 0, nullptr, 0, nullptr, 1, &barrier);

		if (!EndSingleUseCommand(layoutTransitionCommandBuffer))
		{
			Logger::Log("Could not end command buffer for image layout transition.");
			return false;
		}

		return true;
	}

	bool TextureManager::CopyStagingBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
	{
		VkCommandBuffer copyCommandBuffer;
		if (!CreateSingleUseCommand(copyCommandBuffer))
		{
			Logger::Log("Could not create single use command buffer for copying buffer to image.");
			return false;
		}

		VkBufferImageCopy copyRegion = {};
		copyRegion.bufferOffset = 0;
		copyRegion.bufferRowLength = 0;
		copyRegion.bufferImageHeight = 0;
		copyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		copyRegion.imageSubresource.mipLevel = 0;
		copyRegion.imageSubresource.baseArrayLayer = 0;
		copyRegion.imageSubresource.layerCount = 1;
		copyRegion.imageOffset = { 0, 0, 0 };
		copyRegion.imageExtent = { width, height, 1 };

		vkCmdCopyBufferToImage(copyCommandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);

		if (!EndSingleUseCommand(copyCommandBuffer))
		{
			Logger::Log("Could not end single use command buffer for copying buffer to image.");
			return false;
		}

		return true;
	}

	bool TextureManager::CreateSingleUseCommand(VkCommandBuffer& commandBuffer)
	{
		VkCommandBufferAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.pNext = nullptr;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = m_singleUseBufferCommandPool;
		allocInfo.commandBufferCount = 1;
		VkResult result = vkAllocateCommandBuffers(Device::Get().m_device, &allocInfo, &commandBuffer);
		if (result != VK_SUCCESS)
		{
			Logger::Log("Could not allocate single use command buffer");
			return false;
		}

		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.pNext = nullptr;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		beginInfo.pInheritanceInfo = nullptr;
		result = vkBeginCommandBuffer(commandBuffer, &beginInfo);
		if (result != VK_SUCCESS)
		{
			Logger::Log("Could not begin single use command buffer");
			return false;
		}

		return true;
	}

	bool TextureManager::EndSingleUseCommand(VkCommandBuffer& commandBuffer)
	{
		VkResult result = vkEndCommandBuffer(commandBuffer);
		if (result != VK_SUCCESS)
		{
			Logger::Log("Could not end command buffer for copying staging buffer.");
			return false;
		}

		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.pNext = nullptr;
		submitInfo.waitSemaphoreCount = 0;
		submitInfo.pWaitSemaphores = nullptr;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;
		submitInfo.signalSemaphoreCount = 0;
		submitInfo.pSignalSemaphores = nullptr;

		result = vkQueueSubmit(Device::Get().m_multipurposeQueue, 1, &submitInfo, VK_NULL_HANDLE);
		if (result != VK_SUCCESS)
		{
			Logger::Log("Could not submit queue for copying staging buffer.");
			return false;
		}
		result = vkQueueWaitIdle(Device::Get().m_multipurposeQueue);
		if (result != VK_SUCCESS)
		{
			Logger::Log("Could not wait for idle queue for copying staging buffer.");
			return false;
		}

		vkFreeCommandBuffers(Device::Get().m_device, m_singleUseBufferCommandPool, 1, &commandBuffer);

		return true;
	}

	//helper function from Vulkan Samples
	bool TextureManager::FindMemoryTypeFromProperties(uint32_t typeBits, VkFlags requirements_mask, uint32_t* typeIndex)
	{
		// Search memtypes to find first index with those properties
		for (uint32_t i = 0; i < Device::Get().m_physicalDeviceMemoryProperties.memoryTypeCount; i++) {
			if (typeBits & (1 << i)) {
				// Type is available, does it match user properties?
				if ((Device::Get().m_physicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & requirements_mask) == requirements_mask) {
					*typeIndex = i;
					return true;
				}
			}
		}
		// No memory types matched, return failure
		return false;
	}
	uint32_t TextureManager::GetNumberTextures()
	{
		return static_cast<uint32_t>(m_textures.size());
	}
	VkDescriptorImageInfo* TextureManager::GetDescriptorImageInfo()
	{
		return m_textureInfos.data();
	}
}