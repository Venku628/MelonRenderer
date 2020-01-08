#pragma once

#include "Basics.h"
#include "Drawable.h"
#include "Texture.h"

#include <vector>

namespace MelonRenderer
{
	class Pipeline
	{

		//Renderpass
		//---------------------------------------
		VkRenderPass m_renderPass;

		bool CreateRenderPass();
		//---------------------------------------

		//---------------------------------------
		VkSwapchainKHR m_swapchain;
		std::vector<VkImage> m_swapchainImages;
		std::vector<VkImageView> m_swapchainImageViews;
		VkPresentModeKHR m_presentMode;
		uint32_t m_currentImageIndex;
		VkSemaphore m_semaphore;
		VkFence m_fence;
		bool AquireNextImage();
		bool PresentImage();
		//---------------------------------------

		//---------------------------------------
		VkImage m_depthBuffer;
		VkDeviceMemory m_depthBufferMemory;
		VkImageView m_depthBufferView;
		VkFormat m_depthBufferFormat;

		bool CreateDepthBuffer();
		bool FindMemoryTypeFromProperties(uint32_t typeBits, VkFlags requirements_mask, uint32_t* typeIndex);
		//---------------------------------------

		//---------------------------------------
		mat4 m_modelViewProjection;
		VkBuffer m_uniformBuffer;
		VkDeviceMemory m_uniformBufferMemory;

		bool CreateUniformBufferMVP();
		//---------------------------------------

		//shader modules
		//---------------------------------------
		//this struct holds the shader modules
		std::vector<VkPipelineShaderStageCreateInfo> m_shaderStagesV;

		bool CreateShaderModule(const std::vector<char>& code, VkShaderModule& shaderModule);
		bool CreateShaderModules();
		//---------------------------------------

		//frame buffers
		//---------------------------------------
		std::vector<VkFramebuffer> m_framebuffers;

		bool CreateFramebuffers();
		//---------------------------------------


		//vertex buffer
		//---------------------------------------
		std::vector<VkVertexInputAttributeDescription> m_vertexInputAttributes;
		std::vector<VkVertexInputBindingDescription> m_vertexInputBindings;
		//---------------------------------------

		//drawables
		//---------------------------------------
		std::vector<Drawable> m_drawables;
		bool CreateDrawableBuffers(Drawable& drawable);
		bool CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
		//---------------------------------------

		//---------------------------------------
		VkPipeline m_pipeline;

		bool CreateGraphicsPipeline();
		//---------------------------------------

		bool Draw();
		uint32_t m_imageIndex;
		VkViewport m_viewport;
		VkRect2D m_scissorRect2D;

		unsigned int m_windowWidth, m_windowHeight;

		//---------------------------------------
		VkDescriptorPool m_descriptorPool;
		std::vector<VkDescriptorSet> m_descriptorSets;
		VkDescriptorBufferInfo m_descriptorBufferInfoViewProjection;
		std::vector<VkDescriptorSetLayout> m_descriptorSetLayouts;
		VkPipelineLayout m_pipelineLayout;
		bool CreatePipelineLayout();
		bool CreateDescriptorPool();
		bool CreateDescriptorSet();
		//---------------------------------------

		// temporarily only one of each
		//---------------------------------------
		const uint32_t m_queueFamilyIndex = 0; // debug for this system until requirements defined
		VkQueue m_multipurposeQueue;
		VkCommandPool m_multipurposeCommandPool;
		VkCommandBuffer m_multipurposeCommandBuffer;
		//---------------------------------------


		

		//staging buffer
		//---------------------------------------
		VkCommandPool m_singleUseBufferCommandPool;
		//TODO: evaluate bulk copying of buffers within one commandbuffer, within one tick or init
		bool CopyStagingBufferToBuffer(VkBuffer cpuVisibleBuffer, VkBuffer gpuOnlyBuffer, VkDeviceSize size);
		bool CreateOptimalBuffer(VkBuffer& buffer, VkDeviceMemory& bufferMemory, const void* data, VkDeviceSize bufferSize, VkBufferUsageFlagBits bufferUsage);
		bool CreateSingleUseCommand(VkCommandBuffer& commandBuffer);
		bool EndSingleUseCommand(VkCommandBuffer& commandBuffer);
		//---------------------------------------

		friend class Renderer;
	};
}