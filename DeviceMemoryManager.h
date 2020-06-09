#pragma once

#include <vector>
#include <unordered_map>

#include "Basics.h"
#include "Texture.h"

namespace MelonRenderer
{
	struct DynamicUniformBuffer
	{
		VkBuffer m_buffer = VK_NULL_HANDLE;
		VkDeviceMemory m_bufferMemory = VK_NULL_HANDLE;
		VkDescriptorBufferInfo m_descriptorBufferInfo;
		VkDeviceSize m_size = 0;
		VkDeviceSize m_alignment = 0;
		uint32_t m_numberOfElements = 0;
		void* m_uploadBuffer = nullptr;
	};


	class DeviceMemoryManager
	{
	protected:
		std::unordered_map<std::string, uint32_t> m_textureIDs;
		std::vector<Texture> m_textures;
		std::vector<VkDescriptorImageInfo> m_textureInfos;

		VkSampler m_textureSampler;
		VkCommandPool m_singleUseBufferCommandPool;

		VkPhysicalDeviceMemoryProperties m_physicalDeviceMemoryProperties;
		VkPhysicalDeviceProperties m_physicalDeviceProperties;

	public:
		bool Init(VkPhysicalDeviceMemoryProperties& physicalDeviceMemoryProperties, VkPhysicalDeviceProperties& physicalDeviceProperties);
		~DeviceMemoryManager();

		bool CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory) const;
		bool CreateOptimalBuffer(VkBuffer& buffer, VkDeviceMemory& bufferMemory, const void* data, VkDeviceSize bufferSize, VkBufferUsageFlags bufferUsage) const;
		bool UpdateOptimalBuffer(VkBuffer& buffer, const void* data, VkDeviceSize bufferSize) const;
		bool CopyDataToMemory(VkDeviceMemory& memory, void* data, VkDeviceSize dataSize) const;

		uint32_t CreateTextureID(const char* fileName);
		bool CreateImage(VkImage& image, VkDeviceMemory& imageMemory, VkExtent2D& extent, VkImageUsageFlags usage);
		bool CreateImageView(VkImageView& imageView, VkImage image);
		bool CreateTextureImage(VkImage& texture, VkDeviceMemory& textureMemory, unsigned char* pixelData, int width, int height);
		bool CreateTexture(const char* fileName);
		bool CreateTextureSampler();
		bool TransitionImageLayout(VkCommandBuffer& commandBuffer, VkImage image, VkImageLayout previousLayout, VkImageLayout desiredLayout, 
			VkPipelineStageFlags srcStageFlags = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VkPipelineStageFlags dstStageFlags = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
		
		bool CopyStagingBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) const;
		bool CopyStagingBufferToBuffer(VkBuffer cpuVisibleBuffer, VkBuffer gpuOnlyBuffer, VkDeviceSize size) const;

		bool CreateSingleUseCommand(VkCommandBuffer& commandBuffer) const;
		bool EndSingleUseCommand(VkCommandBuffer& commandBuffer) const;

		bool FindMemoryTypeFromProperties(uint32_t typeBits, VkFlags requirements_mask, uint32_t* typeIndex) const;

		uint32_t GetNumberTextures();
		VkDescriptorImageInfo* GetDescriptorImageInfo();

		//TODO: finish rework
		bool CreateDynamicUBO(DynamicUniformBuffer& dynamicUniformBuffer);
		bool UpdateDynamicUBO(DynamicUniformBuffer& dynamicUniformBuffer);
	};

}