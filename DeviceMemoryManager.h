#pragma once

#include <vector>

#include "Basics.h"
#include "Texture.h"

namespace MelonRenderer
{

	class DeviceMemoryManager
	{
	protected:
		std::vector<Texture> m_textures;
		std::vector<VkDescriptorImageInfo> m_textureInfos;

		VkSampler m_textureSampler;
		VkCommandPool m_singleUseBufferCommandPool;

		VkBuffer m_dynTransformUBO;
		VkDeviceMemory m_dynTransformUBOMemory;
		VkDescriptorBufferInfo m_dynTransformUBODescriptorInfo;
		void* m_dynTransformUBOData;
		mat3x4* m_dynTransformMats;
		VkDeviceSize m_dynTransformUBOSize;
		VkDeviceSize m_dynTransformUBOAllignment;
		uint32_t m_maxNumberOfTransforms;
		std::vector<mat3x4>* m_inputTransforms;

		VkPhysicalDeviceMemoryProperties m_physicalDeviceMemoryProperties;

	public:
		bool Init(VkPhysicalDeviceMemoryProperties& physicalDeviceMemoryProperties);

		bool CopyDataToMemory(VkDeviceMemory& memory, void* data, VkDeviceSize dataSize) const;
		bool CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory) const;
		bool CreateOptimalBuffer(VkBuffer& buffer, VkDeviceMemory& bufferMemory, const void* data, VkDeviceSize bufferSize, VkBufferUsageFlagBits bufferUsage) const;

		bool CreateDynTransformUBO(uint32_t numberOfTransforms);
		bool UpdateDynTransformUBO();
		void SetDynamicUBOAlignment(size_t alignment);
		size_t GetDynamicUBOAlignment();
		void SetDynTransformMats(std::vector<mat3x4>* transformMats);
		VkDescriptorBufferInfo* GetDynamicTransformDescriptor();

		bool CreateImage(VkImage& image, VkDeviceMemory& imageMemory, VkExtent2D& extent, VkImageUsageFlags usage);
		bool CreateTextureImage(VkImage& texture, VkDeviceMemory& textureMemory, unsigned char* pixelData, int width, int height);
		bool CreateImageView(VkImageView& imageView, VkImage image);
		bool CreateTexture(const char* filePath);
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

	};

}