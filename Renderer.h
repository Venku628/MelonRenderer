#pragma once

#include "Basics.h"
#include "DeviceMemoryManager.h"
#include "pipelines/PipelineRasterization.h"
#include "pipelines/PipelineRaytracing.h"
#include "pipelines/PipelineImGui.h"
#include "Swapchain.h"
#include "simple_scene_graph/Scene.h"

#include <glfw3.h>
#include "imgui/imgui.h"

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
	const unsigned int defaultWidth = 1280, defaultHeight = 720;

	void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
	void ScrollCallback(GLFWwindow* window, double xOffset, double yOffset);
	void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
	void CharCallback(GLFWwindow* window, unsigned int c);

	static const char* GetClipboardText(void* user_data)
	{
		return glfwGetClipboardString((GLFWwindow*)user_data);
	}

	static void SetClipboardText(void* user_data, const char* text)
	{
		glfwSetClipboardString((GLFWwindow*)user_data, text);
	}

	struct GlfwInputData
	{
		bool m_mouseButtonPressed[5] = { false, false, false, false, false };
		GLFWcursor* m_mouseCursors[ImGuiMouseCursor_COUNT] = {};

		GLFWmousebuttonfun m_prevUserCallbackMousebutton = NULL;
		GLFWscrollfun m_prevUserCallbackScroll = NULL;
		GLFWkeyfun m_prevUserCallbackKey = NULL;
		GLFWcharfun m_prevUserCallbackChar = NULL;
	};

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

		//renderpass for output 
		bool CreateRenderpass();
		bool BeginRenderpass(VkCommandBuffer& commandBuffer);
		bool BeginCommandBuffer(VkCommandBuffer& commandBuffer);
		bool EndRenderpass(VkCommandBuffer& commandBuffer);
		bool EndCommandBuffer(VkCommandBuffer& commandBuffer);
		VkRenderPass m_renderpass;

		//
		bool CopyOutputToSwapchain(VkCommandBuffer& commandBuffer, VkImage storage);

		bool Resize();

		//input
		//-------------------------------------
		bool GlfwInputInit();
		bool GlfwInputTick();
		bool GlfwUpdateMousePosAndButtons();
		bool GlfwUpdateMouseCursor();
		bool GlfwUpdateGamepads();
		
		GlfwInputData m_inputData;
		//-------------------------------------

		VkPresentModeKHR m_presentMode;
		const uint32_t m_queueFamilyIndex = 0;
		VkExtent2D m_extent;

		Swapchain m_swapchain;

		PipelineRaytracing m_raytracingPipeline;
		PipelineRasterization m_rasterizationPipeline;
		PipelineImGui m_imguiPipeline;

		Camera m_camera;
		Scene m_scene;
		DeviceMemoryManager m_memoryManager;

		std::vector<mat3x4> m_transformMats;

		Drawable m_drawable;
		std::vector<NodeDrawable> m_drawableNodes;

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
		VkPhysicalDeviceProperties2 m_currentPhysicalDeviceProperties2;
		VkSurfaceCapabilitiesKHR m_currentSurfaceCapabilities;
		std::vector<VkPresentModeKHR> m_currentPhysicalDevicePresentModes;
		std::vector<VkQueueFamilyProperties> m_currentQueueFamilyProperties;

		VkPhysicalDeviceRayTracingPropertiesNV m_raytracingProperties;
		bool m_hasRaytracingCapabilities;

		GLFWwindow* m_window;
		VkSurfaceKHR m_presentationSurface;
		VkPhysicalDeviceMemoryProperties m_physicalDeviceMemoryProperties;
		VkInstance m_vulkanInstance;
		VULKAN_LIBRARY_TYPE m_vulkanLibrary;
	};
}
