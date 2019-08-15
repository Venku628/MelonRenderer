


//TODO: figure out if xcb or xlib use platform needs to be defined

#include "Window.h"
#include "loader/VulkanFunctions.h"
#include "Logger.h"

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
	class Renderer
	{
	public:
		void Init();

		void Fini();

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

		std::vector<char const *> m_requiredInstanceExtensions;
		std::vector<const char*> m_requiredDeviceExtensions;
		std::vector<VkPhysicalDevice> m_physicalDevices;

		VkPhysicalDeviceFeatures m_currentPhysicalDeviceFeatures;
		VkPhysicalDeviceProperties m_currentPhysicalDeviceProperties;
		std::vector<VkQueueFamilyProperties> m_currentQueueFamilyProperties;

		// temporarily only one
		VkQueue m_multipurposeQueue;

		WindowHandle m_windowHandle;
		VkSurfaceKHR m_presentationSurface;
		VkDevice m_logicalDevice;
		VkInstance m_vulkanInstance;
		VULKAN_LIBRARY_TYPE m_vulkanLibrary;
		
	};
}



