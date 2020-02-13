#pragma once

#include "../Basics.h"
#include "../Drawable.h"
#include "../Shader.h"
#include "../DeviceMemoryManager.h"

#include <vector>

namespace MelonRenderer
{
	struct OutputSurface
	{
		VkSurfaceCapabilitiesKHR capabilites;
		VkSurfaceKHR surface;
	};

	class Pipeline
	{
	public:
		virtual void Init(VkPhysicalDevice& physicalDevice, DeviceMemoryManager& memoryManager, OutputSurface outputSurface, VkExtent2D windowExtent) = 0;
		virtual void Tick(float timeDelta) = 0;
		virtual void RecreateSwapchain(VkExtent2D windowExtent) = 0;

		virtual void Fini() = 0;

	protected:
		virtual void DefineVertices() = 0;

		virtual bool CreateSwapchain(VkPhysicalDevice& device) = 0;
		bool CleanupSwapchain();

		bool CreateCommandBufferPool(VkCommandPool& commandPool, VkCommandPoolCreateFlags flags);
		bool CreateCommandBuffer(VkCommandPool& commandPool, VkCommandBuffer& commandBuffer);

		const uint32_t m_numberOfSamples = 1;
		VkExtent2D m_extent;

		OutputSurface m_outputSurface;

		VkPhysicalDevice* m_physicalDevice;

		//---------------------------------------
		VkSwapchainKHR m_swapchain;
		std::vector<VkImage> m_swapchainImages;
		std::vector<VkImageView> m_swapchainImageViews;
		VkPresentModeKHR m_presentMode;
		uint32_t m_currentImageIndex;
		VkSemaphore m_semaphore;
		VkFence m_fence;
		bool AquireNextImage();
		bool PresentImage(VkFence* drawFence = nullptr);
		//---------------------------------------

		//Renderpass
		//---------------------------------------
		VkRenderPass m_renderPass;

		virtual bool CreateRenderPass() = 0;
		//---------------------------------------


		//shader modules
		//---------------------------------------
		//this struct holds the shader modules
		std::vector<VkPipelineShaderStageCreateInfo> m_shaderStagesV;
		//TODO: destroy the shader stages after they are used to create the graphics pipeline

		bool CreateShaderModule(const std::vector<char>& code, VkShaderModule& shaderModule);
		virtual bool CreateShaderModules() = 0;
		//---------------------------------------


		//frame buffers
		//---------------------------------------
		std::vector<VkFramebuffer> m_framebuffers;

		virtual bool CreateFramebuffers() = 0;
		//---------------------------------------


		//vertex buffer
		//---------------------------------------
		std::vector<VkVertexInputAttributeDescription> m_vertexInputAttributes;
		std::vector<VkVertexInputBindingDescription> m_vertexInputBindings;
		//---------------------------------------

		//---------------------------------------
		VkPipeline m_pipeline;

		virtual bool CreateGraphicsPipeline() = 0;
		//---------------------------------------

		virtual bool Draw(float timeDelta) = 0;
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
		virtual bool CreatePipelineLayout() = 0;
		virtual bool CreateDescriptorPool() = 0;
		virtual bool CreateDescriptorSet() = 0;
		//---------------------------------------

		// temporarily only one of each
		//---------------------------------------
		const uint32_t m_queueFamilyIndex = 0; // debug for this system until requirements defined
		VkCommandPool m_multipurposeCommandPool;
		VkCommandBuffer m_multipurposeCommandBuffer;
		//---------------------------------------

		DeviceMemoryManager* m_memoryManager;
	};
}