#pragma once

#include "Basics.h"
#include "DeviceMemoryManager.h"
#include "pipelines/PipelineRasterization.h"

#include <glfw3.h>

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

#if defined _WIN32
#define VULKAN_LIBRARY_TYPE HMODULE
#elif defined __linux
#include <dlfcn.h>
#define VULKAN_LIBRARY_TYPE void*
#endif

namespace MelonRenderer
{
	class Renderer 
	{
	public:
		void Init();
		bool Tick();
		void Loop();
		void Fini();

	private:
		bool CreateGLFWWindow();

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

		bool LoadDeviceFunctions();
		bool LoadDeviceExtensionFunctions();

		bool AquireQueueHandles();
		bool CreatePresentationSurface();
		bool EnumeratePresentationModes(VkPhysicalDevice& device);
		bool SelectPresentationMode(VkPresentModeKHR desiredPresentMode);
		bool CheckPresentationSurfaceCapabilities(VkPhysicalDevice& device);

		bool CreateLogicalDeviceAndQueue(VkPhysicalDevice& device);
		

		VkPresentModeKHR m_presentMode;
		const uint32_t m_queueFamilyIndex = 0;
		VkExtent2D m_extent;
		PipelineRasterization m_rasterizationPipeline;

		DeviceMemoryManager m_memoryManager;

		//time logic
		//---------------------------------------
		std::chrono::time_point<std::chrono::steady_clock> timeLast;
		std::chrono::time_point<std::chrono::steady_clock> timeNow;
		//---------------------------------------

		std::vector<char const*> m_requiredInstanceExtensions;
		std::vector<const char*> m_requiredDeviceExtensions;
		std::vector<VkPhysicalDevice> m_physicalDevices;

		uint32_t m_currentPhysicalDeviceIndex = 0;
		VkPhysicalDeviceFeatures m_currentPhysicalDeviceFeatures;
		VkPhysicalDeviceProperties m_currentPhysicalDeviceProperties;
		VkSurfaceCapabilitiesKHR m_currentSurfaceCapabilities;
		std::vector<VkPresentModeKHR> m_currentPhysicalDevicePresentModes;
		std::vector<VkQueueFamilyProperties> m_currentQueueFamilyProperties;
		bool m_hasRaytracingCapabilities;

		GLFWwindow* m_window;
		VkSurfaceKHR m_presentationSurface;
		VkPhysicalDeviceMemoryProperties m_physicalDeviceMemoryProperties;
		VkInstance m_vulkanInstance;
		VULKAN_LIBRARY_TYPE m_vulkanLibrary;
		
	};
}
