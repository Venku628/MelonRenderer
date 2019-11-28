//TODO: figure out if xcb or xlib use platform needs to be defined
#pragma once

#include "Window.h"
#include "Shader.h"
#include "Basics.h"

#include "Drawable.h"

#include <iostream>
#include <vector>
#include <chrono>

constexpr VkApplicationInfo applicationInfo =
{
	VK_STRUCTURE_TYPE_APPLICATION_INFO,
	nullptr,
	"MelonRendererDemo",
	VK_MAKE_VERSION(0, 1, 0),
	"MelonEngine",
	VK_MAKE_VERSION(0, 1, 0),
	VK_MAKE_VERSION(0, 1, 0)
};

struct QueueFamilyInfo
{
	uint32_t familyIndex = UINT32_MAX;
	std::vector<float> queuePriorities;
};

#define MELON_WINDOW_NAME "MelonRenderer"

namespace MelonRenderer
{
	class Renderer : public WindowEventHandlerBase
	{
	public:
		void Init(WindowHandle& windowHandle);
		bool Tick() override;
		void Fini();

		bool OnWindowSizeChanged() override;

	private:
		bool LoadVulkanLibrary();
		bool LoadExportedFunctions();
		bool LoadGlobalFunctions();
		bool CheckInstanceExtensions();
		bool CreateInstance();
		bool LoadInstanceFunctions();
		bool LoadInstanceExtensionFunctions();
		bool EnumeratePhysicalDevices();

		bool CheckRequiredDeviceExtensions(VkPhysicalDevice & device);
		bool CheckRequiredDeviceFeatures(VkPhysicalDevice& device);
		bool CheckRequiredDeviceProperties(VkPhysicalDevice& device);
		bool CheckQueueFamiliesAndProperties(VkPhysicalDevice& device);
		bool FindCompatibleQueueFamily(VkQueueFlags flags, std::vector<QueueFamilyInfo>& familyIndices);

		void CreateLogicalDeviceAndFollowing(VkPhysicalDevice& device);

		bool LoadDeviceFunctions();
		bool LoadDeviceExtensionFunctions();

		bool AquireQueueHandles();
		bool CreatePresentationSurface();
		bool EnumeratePresentationModes(VkPhysicalDevice& device);
		bool SelectPresentationMode(VkPresentModeKHR desiredPresentMode);
		bool CheckPresentationSurfaceCapabilities(VkPhysicalDevice& device);

		bool CreateSwapchain(VkPhysicalDevice& device);
		bool RecreateSwapchain();
		bool CleanupSwapchain();

		bool CreateCommandBufferPool(VkCommandPool& commandPool, VkCommandPoolCreateFlags flags);
		bool CreateCommandBuffer(VkCommandPool& commandPool, VkCommandBuffer& commandBuffer);

		std::vector<char const *> m_requiredInstanceExtensions;
		std::vector<const char*> m_requiredDeviceExtensions;
		std::vector<VkPhysicalDevice> m_physicalDevices;

		uint32_t m_currentPhysicalDeviceIndex = 0;
		VkPhysicalDeviceFeatures m_currentPhysicalDeviceFeatures;
		VkPhysicalDeviceProperties m_currentPhysicalDeviceProperties;
		VkSurfaceCapabilitiesKHR m_currentSurfaceCapabilities;
		std::vector<VkPresentModeKHR> m_currentPhysicalDevicePresentModes;
		std::vector<VkQueueFamilyProperties> m_currentQueueFamilyProperties;
		bool m_hasRaytracingCapabilities;

		const uint32_t m_numberOfSamples = 1;
		VkExtent3D m_extent;

		//TODO: evaluate seperate class for this purpose, semaphores etc. included
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

	
		//TODO: depth buffer class?
		//---------------------------------------
		VkImage m_depthBuffer;
		VkDeviceMemory m_depthBufferMemory;
		VkImageView m_depthBufferView;
		VkFormat m_depthBufferFormat;

		bool CreateDepthBuffer();
		bool FindMemoryTypeFromProperties(uint32_t typeBits, VkFlags requirements_mask, uint32_t* typeIndex);
		//---------------------------------------


		//uniform buffer class? buffers should also be allocated in bulk 
		//---------------------------------------
		mat4 m_modelViewProjection;
		VkBuffer m_uniformBuffer;
		VkDeviceMemory m_uniformBufferMemory;

		bool CreateUniformBufferMVP();
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

		unsigned int m_windowWidth, m_windowHeight, m_windowX, m_windowY;

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

		//texture
		//---------------------------------------
		VkImage m_textureImage;
		VkDeviceMemory m_textureImageMemory;
		VkImageView m_textureView;
		VkSampler m_textureSampler;
		VkDescriptorImageInfo m_descriptorImageInfoTexture = {}; //TODO: one per swapchain
		bool CreateTexture(const char* filePath);
		bool CreateTextureView();
		bool CreateTextureSampler();

		bool TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout previousLayout, VkImageLayout desiredLayout);
		bool CopyStagingBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
		bool CreateSingleUseCommand(VkCommandBuffer& commandBuffer);
		bool EndSingleUseCommand(VkCommandBuffer& commandBuffer);
		//---------------------------------------

		//staging buffer
		//---------------------------------------
		VkCommandPool m_singleUseBufferCommandPool;
		//TODO: evaluate bulk copying of buffers within one commandbuffer, within one tick or init
		bool CopyStagingBufferToBuffer(VkBuffer cpuVisibleBuffer, VkBuffer gpuOnlyBuffer, VkDeviceSize size);
		bool CreateOptimalBuffer(VkBuffer& buffer, VkDeviceMemory& bufferMemory, const void* data, VkDeviceSize bufferSize, VkBufferUsageFlagBits bufferUsage);
		//---------------------------------------

		//time logic
		//---------------------------------------
		std::chrono::time_point<std::chrono::steady_clock> timeLast;
		std::chrono::time_point<std::chrono::steady_clock> timeNow;
		//---------------------------------------


		VkFormat m_format;
		WindowHandle m_windowHandle;
		VkSurfaceKHR m_presentationSurface;
		VkDevice m_logicalDevice;
		VkPhysicalDeviceMemoryProperties m_physicalDeviceMemoryProperties;
		VkInstance m_vulkanInstance;
		VULKAN_LIBRARY_TYPE m_vulkanLibrary;
		
	};
}
