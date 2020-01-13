#pragma once

#include "Basics.h"
#include "Drawable.h"
#include "Shader.h"
#include "DeviceMemoryManager.h"

#include <vector>

namespace MelonRenderer
{
	class Pipeline
	{
		//to be moved to pipeline class
		//--------------------------------------
	public:
		void Init(VkPhysicalDevice& device, DeviceMemoryManager& memoryManager);
		void Tick(float timeDelta);
		void RecreateSwapchain(unsigned int width, unsigned int height);

		void Fini();

	protected:
		//virtual void     = 0; in pipeline base
		void DefineVertices();
		void InitCam();

		bool CreateSwapchain(VkPhysicalDevice& device);
		bool CleanupSwapchain();

		bool CreateCommandBufferPool(VkCommandPool& commandPool, VkCommandPoolCreateFlags flags);
		bool CreateCommandBuffer(VkCommandPool& commandPool, VkCommandBuffer& commandBuffer);

		const uint32_t m_numberOfSamples = 1;
		VkExtent3D m_extent;

		VkPhysicalDevice& m_physicalDevice;

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

		//Renderpass
		//---------------------------------------
		VkRenderPass m_renderPass;

		bool CreateRenderPass();
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
		//---------------------------------------

		//---------------------------------------
		VkPipeline m_pipeline;

		bool CreateGraphicsPipeline();
		//---------------------------------------

		bool Draw(float timeDelta);
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
		VkCommandPool m_multipurposeCommandPool;
		VkCommandBuffer m_multipurposeCommandBuffer;
		//---------------------------------------

		//everything that should be partially moved to the memoryManager
		//---------------------------------------
		DeviceMemoryManager& m_memoryManager;

		//TODO: depth buffer class?
		VkImage m_depthBuffer;
		VkDeviceMemory m_depthBufferMemory;
		VkImageView m_depthBufferView;
		VkFormat m_depthBufferFormat;

		bool CreateDepthBuffer();
		bool FindMemoryTypeFromProperties(uint32_t typeBits, VkFlags requirements_mask, uint32_t* typeIndex);

		//uniform buffer class? buffers should also be allocated in bulk 
		mat4 m_modelViewProjection;
		VkBuffer m_uniformBuffer;
		VkDeviceMemory m_uniformBufferMemory;

		bool CreateUniformBufferMVP();
		//---------------------------------------
		

		
	};
}