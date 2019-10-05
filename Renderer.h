


//TODO: figure out if xcb or xlib use platform needs to be defined

#include "Window.h"
#include "loader/VulkanFunctions.h"
#include "Logger.h"
#include "MelonMath.h"

#include <iostream>
#include <vector>

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

		bool CreateCommandBufferPool(VkCommandPool& commandPool);
		bool CreateCommandBuffer(VkCommandPool& commandPool, VkCommandBuffer commandBuffer);


		std::vector<char const *> m_requiredInstanceExtensions;
		std::vector<const char*> m_requiredDeviceExtensions;
		std::vector<VkPhysicalDevice> m_physicalDevices;

		uint32_t m_currentPhysicalDeviceIndex = 0;
		VkPhysicalDeviceFeatures m_currentPhysicalDeviceFeatures;
		VkPhysicalDeviceProperties m_currentPhysicalDeviceProperties;
		VkSurfaceCapabilitiesKHR m_currentSurfaceCapabilities;
		std::vector<VkPresentModeKHR> m_currentPhysicalDevicePresentModes;
		std::vector<VkQueueFamilyProperties> m_currentQueueFamilyProperties;

		const uint32_t m_numberOfSamples = 1;
		VkExtent3D m_extent;

		//TODO: evaluate seperate class for this purpose, semaphores etc. included
		//---------------------------------------
		VkSwapchainKHR m_swapchain;
		std::vector<VkImage> m_swapchainImages;
		std::vector<VkImageView> m_imageViews;
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

		// TODO: uniform buffer class? buffers should also be allocated in bulk 
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

		VkDescriptorPool m_descriptorPool;
		VkDescriptorSet m_descriptorSet;
		VkDescriptorBufferInfo m_descriptorBufferInfo;
		VkDescriptorSetLayout m_uniformBufferDescriptorSetLayout;
		VkPipelineLayout m_pipelineLayout;
		bool CreatePipelineLayout();
		bool CreateDescriptorPool();
		bool CreateDescriptorSet();
		
		// temporarily only one of each
		const uint32_t m_queueFamilyIndex = 0; // debug for this system until requirements defined
		VkQueue m_multipurposeQueue;
		VkCommandPool m_multipurposeCommandPool;
		VkCommandBuffer m_multipurposeCommandBuffer;

		VkFormat m_format;
		WindowHandle m_windowHandle;
		VkSurfaceKHR m_presentationSurface;
		VkDevice m_logicalDevice;
		VkPhysicalDeviceMemoryProperties m_physicalDeviceMemoryProperties;
		VkInstance m_vulkanInstance;
		VULKAN_LIBRARY_TYPE m_vulkanLibrary;
		
	};
}



