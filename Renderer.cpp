#include "Renderer.h"



namespace MelonRenderer
{

	void MelonRenderer::Renderer::Init()
	{
		CreateGLFWWindow();

		timeLast = timeNow = std::chrono::high_resolution_clock::now();

		// TODO: handle failure with termination, maybe std quick exit?

		LoadVulkanLibrary();
		LoadExportedFunctions();
		LoadGlobalFunctions();
		CheckInstanceExtensions();
		CreateInstance();
		LoadInstanceFunctions();
		LoadInstanceExtensionFunctions();
		EnumeratePhysicalDevices();

		m_requiredDeviceExtensions.emplace_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

		// TODO: remember last used device or cycle through devices until one works the first time
		
		for (int i = 0; i < m_physicalDevices.size(); i++)
		{
			if (!CheckRequiredDeviceProperties(m_physicalDevices[i]))
			{
				continue;
			}
			m_currentPhysicalDeviceIndex = i;
			if (m_currentPhysicalDeviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
			{
				break;
			}
		}
		CreateLogicalDeviceAndQueue(m_physicalDevices[m_currentPhysicalDeviceIndex]);

		m_memoryManager.Init(m_physicalDeviceMemoryProperties);

		m_rasterizationPipeline.Init(m_physicalDevices[m_currentPhysicalDeviceIndex], m_memoryManager);


		Logger::Log("Loading complete.");
	}

	bool Renderer::Tick()
	{
		timeNow = std::chrono::high_resolution_clock::now();
		float timeDelta = static_cast<float>(std::chrono::duration_cast<std::chrono::milliseconds>(timeNow - timeLast).count());
		std::string logMessage = "Time passed since last tick: ";
		Logger::Log(logMessage.append(std::to_string(static_cast<unsigned int>(timeDelta))));
		timeLast = timeNow;


		m_rasterizationPipeline.Tick(timeDelta);

		return false;
	}

	void Renderer::Loop()
	{
		while (!glfwWindowShouldClose(m_window))
		{
			glfwPollEvents();

			//TODO: use proper callbacks
			int glfwWidth, glfwHeight;
			glfwGetWindowSize(m_window, &glfwWidth, &glfwHeight);
			if ((glfwWidth != m_extent.width) || (glfwHeight != m_extent.height))
			{
				m_extent.width = glfwWidth;
				m_extent.height = glfwHeight;

				Logger::Log("Recreating swapchain.");
				m_rasterizationPipeline.RecreateSwapchain(m_extent.width, m_extent.height);
			}


			Tick();

			Logger::Get().Print();
		}

	}

	void Renderer::Fini()
	{
		
		vkDestroySurfaceKHR(m_vulkanInstance, m_presentationSurface, nullptr);
		vkDestroyDevice(Device::Get().m_device, nullptr);
		vkDestroyInstance(m_vulkanInstance, nullptr);

#if defined _WIN32
		FreeLibrary(m_vulkanLibrary);
#elif defined __linux
		dlclose(m_vulkanLibrary);
#endif
		m_vulkanLibrary = nullptr;

		glfwDestroyWindow(m_window);
		glfwTerminate();
	}

	bool Renderer::CreateGLFWWindow()
	{
		glfwInit();

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
		m_window = glfwCreateWindow(800, 600, "Vulkan Renderer", nullptr, nullptr);

		if (m_window == nullptr)
		{
			Logger::Log("Could not create glfw window.");
			return false;
		}

		return true;
	}

	bool MelonRenderer::Renderer::LoadVulkanLibrary()
	{
#if defined _WIN32
		m_vulkanLibrary = LoadLibraryA("vulkan-1.dll");
#elif defined __linux
		m_vulkanLibary = dlopen("libvulkan.so.1", RTLD_NOW);
#endif

		if (m_vulkanLibrary == nullptr)
		{
			Logger::Log("Cannot connect to library, consider checking your drivers.");
			return false;
		}

		return true;
	}

	bool MelonRenderer::Renderer::LoadExportedFunctions()
	{
#if defined _WIN32
#define LoadFunction GetProcAddress
#elif defined __linux
#define LoadFunction dlsym
#endif


#define EXPORTED_VULKAN_FUNCTION( name ) name = (PFN_##name)LoadFunction( m_vulkanLibrary, #name);	\
	if (name == nullptr)																		\
	{std::cout << "Could not load exported function named: " #name << std::endl; return false;}	

#include "loader/ListOfVulkanFunctions.inl"

		return true;
	}

	bool MelonRenderer::Renderer::LoadGlobalFunctions()
	{
#define GLOBAL_LEVEL_VULKAN_FUNCTION( name ) name = (PFN_##name)vkGetInstanceProcAddr( nullptr, #name);	\
	if (name == nullptr)																		\
	{std::cout << "Could not load global function named: " #name << std::endl; return false;}	

#include "loader/ListOfVulkanFunctions.inl"

		return true;
	}

	bool Renderer::CheckInstanceExtensions()
	{
		uint32_t extensionCount = 0;
		std::vector<VkExtensionProperties> availableExtensions;

		VkResult result = vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
		if (result != VK_SUCCESS)
		{
			Logger::Log("Could not get number of instance extensions.");
			return false;
		}

		availableExtensions.resize(extensionCount);
		result = vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, &availableExtensions[0]);
		if (result != VK_SUCCESS)
		{
			Logger::Log("Could not enumerate instance extensions.");
			return false;
		}

		//TODO: define all requirements
		m_requiredInstanceExtensions.emplace_back(VK_KHR_SURFACE_EXTENSION_NAME);
		m_requiredInstanceExtensions.emplace_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);

#if defined VK_USE_PLATFORM_WIN32_KHR
		m_requiredInstanceExtensions.emplace_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#elif defined VK_USE_PLATFORM_XCB_KHR
		m_requiredInstanceExtensions.emplace_back(VK_KHR_XCB_SURFACE_EXTENSION_NAME);
#elif defined VK_USE_PLATFORM_XLIB_KHR
		m_requiredInstanceExtensions.emplace_back(VK_KHR_XLIB_SURFACE_EXTENSION_NAME);
#endif 

		for (auto requiredExtension : m_requiredInstanceExtensions)
		{
			bool available = false;
			for (auto availableExtension : availableExtensions)
			{
				if (!strcmp(availableExtension.extensionName, requiredExtension))
				{
					available = true;
					break;
				}
			}
			if (!available)
			{
				std::string logMessage{ "Extension: " };
				logMessage.append(requiredExtension);
				logMessage.append(" not supported.");
				Logger::Log(logMessage);
				return false;
			}
		}

		return true;
	}

	bool Renderer::CreateInstance()
	{
		VkInstanceCreateInfo instanceCreateInfo ={};
		instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		instanceCreateInfo.pNext = nullptr;
		instanceCreateInfo.flags = 0;
		instanceCreateInfo.pApplicationInfo = &applicationInfo;
		instanceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(m_requiredInstanceExtensions.size());
		instanceCreateInfo.ppEnabledExtensionNames = m_requiredInstanceExtensions.size() > 0 ? &m_requiredInstanceExtensions[0] : nullptr;

#ifdef _DEBUG
		std::vector<const char*> validationLayers = {
			"VK_LAYER_KHRONOS_validation"
		};
		instanceCreateInfo.enabledLayerCount = static_cast<unsigned int>(validationLayers.size());
		instanceCreateInfo.ppEnabledLayerNames = validationLayers.data();
		Logger::Log("Activated validation layers for debugging.");
#elif
		instanceCreateInfo.enabledLayerCount = 0;
		instanceCreateInfo.ppEnabledLayerNames = nullptr;
#endif

		VkResult result = vkCreateInstance(&instanceCreateInfo, nullptr, &m_vulkanInstance);
		if ((result != VK_SUCCESS) || (m_vulkanInstance == VK_NULL_HANDLE))
		{
			Logger::Log("Could not create vulkan instance.");
			return false;
		}

		return true;
	}

	bool Renderer::LoadInstanceFunctions()
	{
#define INSTANCE_LEVEL_VULKAN_FUNCTION( name ) name = (PFN_##name)vkGetInstanceProcAddr( m_vulkanInstance, #name);	\
	if (name == nullptr)																		\
	{std::cout << "Could not load instance function named: " #name << std::endl; return false;}	

#include "loader/ListOfVulkanFunctions.inl"

		return true;
	}

	bool Renderer::LoadInstanceExtensionFunctions()
	{
#define INSTANCE_LEVEL_VULKAN_FUNCTION_FROM_EXTENSION( name, extension )													\
for(auto & requiredExtension : m_requiredInstanceExtensions){ if(std::string(requiredExtension) == std::string(extension))	\
		{name = (PFN_##name)vkGetInstanceProcAddr( m_vulkanInstance, #name);												\
	if (name == nullptr)																									\
	{std::cout << "Could not load instance function named: " #name << std::endl; return false;}}}							\

#include "loader/ListOfVulkanFunctions.inl"

		return true;
	}

	bool Renderer::EnumeratePhysicalDevices()
	{
		uint32_t deviceCount = 0;
		
		VkResult result = vkEnumeratePhysicalDevices(m_vulkanInstance, &deviceCount, nullptr);
		if (result != VK_SUCCESS)
		{
			Logger::Log("Could not get the number of physical devices.");
			return false;
		}

		m_physicalDevices.resize(deviceCount);
		result = vkEnumeratePhysicalDevices(m_vulkanInstance, &deviceCount, &m_physicalDevices[0]);
		if (result != VK_SUCCESS)
		{
			Logger::Log("Could not enumerate physical devices.");
			return false;
		}

		return true;
	}

	bool Renderer::CheckRequiredDeviceExtensions(VkPhysicalDevice& device)
	{
		std::vector<VkExtensionProperties> deviceExtensions;

		uint32_t extensionCount = 0;

		VkResult result = vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
		if (result != VK_SUCCESS)
		{
			Logger::Log("Could not get the number of device extensions.");
			return false;
		}

		deviceExtensions.resize(extensionCount);
		result = vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, &deviceExtensions[0]);
		if (result != VK_SUCCESS)
		{
			Logger::Log("Could not enumerate device extensions.");
			return false;
		}

		for (auto& requiredDeviceExtension : m_requiredDeviceExtensions)
		{
			bool extensionSupported = false;

			for (auto& deviceExtension : deviceExtensions)
			{
				if (!strcmp(deviceExtension.extensionName, requiredDeviceExtension))
				{
					extensionSupported = true;
					break;
				}
			}

			if (!extensionSupported)
			{
				std::string logMessage = "Device does not support extension ";
				logMessage.append(requiredDeviceExtension);
				Logger::Log(logMessage);
				return false;
			}
		}

		//check raytracing capabilities at this point because it´s convenient
		m_hasRaytracingCapabilities = false;
		for (auto& deviceExtension : deviceExtensions)
		{
			if (!strcmp(deviceExtension.extensionName, "VK_NV_ray_tracing"))
			{
				Logger::Log("Device supports raytracing!");
				m_hasRaytracingCapabilities = true;
			}
		}

		return true;
	}

	bool Renderer::CheckRequiredDeviceFeatures(VkPhysicalDevice& device)
	{
		vkGetPhysicalDeviceFeatures(device, &m_currentPhysicalDeviceFeatures);

		//TODO: determine if any features are required
		/*
		if (m_currentPhysicalDeviceFeatures.)
		{
			Logger::Log("Could not get physical device features.");
			return false;
		}
		*/

		return true;
	}

	bool Renderer::CheckRequiredDeviceProperties(VkPhysicalDevice& device)
	{
		vkGetPhysicalDeviceProperties(device, &m_currentPhysicalDeviceProperties);

		//TODO: determine if any properties are required
		/*
		if (m_currentPhysicalDeviceProperties.)
		{
			Logger::Log("Could not get physical device properties.");
			return false;
		}
		*/

		return true;
	}

	bool Renderer::CheckQueueFamiliesAndProperties(VkPhysicalDevice& device)
	{
		uint32_t queueFamilyCount = 0;

		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
		if (queueFamilyCount == 0)
		{
			Logger::Log("Could not get the number of queue families.");
			return false;
		}

		m_currentQueueFamilyProperties.resize(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, &m_currentQueueFamilyProperties[0]);
		if (m_currentQueueFamilyProperties.size() == 0)
		{
			Logger::Log("Could not get the queue family propterties.");
			return false;
		}

		return true;
	}

	bool Renderer::FindCompatibleQueueFamily(VkQueueFlags flags, std::vector<QueueFamilyInfo>& familyIndices)
	{
		bool foundCompatibleQueueFamily = false;
		for (uint32_t currentFamilyIndex = 0; currentFamilyIndex < m_currentQueueFamilyProperties.size(); currentFamilyIndex++)
		{
			if (m_currentQueueFamilyProperties[currentFamilyIndex].queueFlags & flags)
			{
				QueueFamilyInfo familyInfo;
				familyInfo.familyIndex = currentFamilyIndex;

				// this is the case for my gpu
				//TODO: find out how to best prioritize queue families on gpus
				if(currentFamilyIndex == m_queueFamilyIndex)
					familyInfo.queuePriorities.resize(m_currentQueueFamilyProperties[currentFamilyIndex].queueCount, 1.0);
				else
					familyInfo.queuePriorities.resize(m_currentQueueFamilyProperties[currentFamilyIndex].queueCount, 0.5);

				familyIndices.emplace_back(familyInfo);
				foundCompatibleQueueFamily = true;
			}
		}

		if (!foundCompatibleQueueFamily)
		{
			Logger::Log("Could not find compatible queueFamily");
			return false;
		}

		return true;
	}

	bool Renderer::CreateLogicalDeviceAndQueue(VkPhysicalDevice& device)
	{
		CheckRequiredDeviceExtensions(device);
		CheckRequiredDeviceFeatures(device);
		CheckQueueFamiliesAndProperties(device);

		vkGetPhysicalDeviceMemoryProperties(device, &m_physicalDeviceMemoryProperties);

		//TODO: define important queue family flags
		constexpr VkQueueFlags basicFlags = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT |
			VK_QUEUE_TRANSFER_BIT | VK_QUEUE_SPARSE_BINDING_BIT;
		std::vector<QueueFamilyInfo> basicQueueFamilyInfos;
		FindCompatibleQueueFamily(basicFlags, basicQueueFamilyInfos);

		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
		for (auto& basicQueueFamilyInfo : basicQueueFamilyInfos)
		{
			VkDeviceQueueCreateInfo queueCreateInfo = {};
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.pNext = nullptr;
			queueCreateInfo.flags = 0;
			queueCreateInfo.queueFamilyIndex = basicQueueFamilyInfo.familyIndex;
			queueCreateInfo.queueCount = static_cast<uint32_t>(basicQueueFamilyInfo.queuePriorities.size());
			queueCreateInfo.pQueuePriorities = &basicQueueFamilyInfo.queuePriorities[0];

			queueCreateInfos.emplace_back(queueCreateInfo);
		}

		//TODO: copy and add raytracing to required extensions
		auto deviceExtensionsToActivate = m_requiredDeviceExtensions;
		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions;
		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
		//deviceExtensionsToActivate.emplace_back(&glfwExtensions);

		if (m_hasRaytracingCapabilities)
		{
			deviceExtensionsToActivate.emplace_back("VK_KHR_get_memory_requirements2");
			deviceExtensionsToActivate.emplace_back("VK_NV_ray_tracing");
		}

		VkPhysicalDeviceFeatures2 features2 = {};
		features2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
		vkGetPhysicalDeviceFeatures2(device, &features2);

		VkDeviceCreateInfo deviceCreateInfo = {};
		deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		deviceCreateInfo.pNext = &features2;
		deviceCreateInfo.flags = 0;
		deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
		deviceCreateInfo.pQueueCreateInfos = &queueCreateInfos[0];
		deviceCreateInfo.enabledLayerCount = 0;
		deviceCreateInfo.ppEnabledLayerNames = nullptr;
		deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensionsToActivate.size());
		deviceCreateInfo.ppEnabledExtensionNames = deviceExtensionsToActivate.data();
		deviceCreateInfo.pEnabledFeatures = nullptr; //raytracing? previously &m_currentPhysicalDeviceFeatures


		VkResult result = vkCreateDevice(device, &deviceCreateInfo, nullptr, &Device::Get().m_device);
		if (result != VK_SUCCESS)
		{
			Logger::Log("Could not create logical device.");
			return false;
		}

		LoadDeviceFunctions();
		LoadDeviceExtensionFunctions();
		AquireQueueHandles();
		CreatePresentationSurface();
		EnumeratePresentationModes(device);
		SelectPresentationMode(VK_PRESENT_MODE_FIFO_KHR);

		CheckPresentationSurfaceCapabilities(device);

	}

	bool Renderer::LoadDeviceFunctions()
	{
#define DEVICE_LEVEL_VULKAN_FUNCTION( name ) name = (PFN_##name)vkGetDeviceProcAddr( Device::Get().m_device, #name);	\
	if (name == nullptr)																						\
	{std::cout << "Could not load device function named: " #name << std::endl; return false;}					\

#include "loader/ListOfVulkanFunctions.inl"

		return true;
	}

	bool Renderer::LoadDeviceExtensionFunctions()
	{
#define DEVICE_LEVEL_VULKAN_FUNCTION_FROM_EXTENSION( name, extension ) name = (PFN_##name)vkGetDeviceProcAddr( Device::Get().m_device, #name);	\
	for(auto & requiredExtension : m_requiredDeviceExtensions){ if(std::string(requiredExtension) == std::string(extension))			\
		{name = (PFN_##name)vkGetDeviceProcAddr( Device::Get().m_device, #name);																\
	if (name == nullptr)																												\
	{std::cout << "Could not load device function named: " #name << std::endl; return false;}}}											\

#include "loader/ListOfVulkanFunctions.inl"

		return true;
	}

	bool Renderer::AquireQueueHandles()
	{
		//TODO: define queue requirements and aquire multiple handles accordingly

		vkGetDeviceQueue(Device::Get().m_device, 0, 0, &Device::Get().m_multipurposeQueue);


		return true;
	}

	bool Renderer::CreatePresentationSurface()
	{
		VkResult result = glfwCreateWindowSurface(m_vulkanInstance, m_window, nullptr, &m_presentationSurface);
		if (result != VK_SUCCESS)
		{
			Logger::Log("Could not create presentation surface.");
			return false;
		}
		return true;
	}

	bool Renderer::EnumeratePresentationModes(VkPhysicalDevice& device)
	{
		uint32_t presentModesCount = 0;

		VkResult result = vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_presentationSurface, &presentModesCount, nullptr);
		if (result != VK_SUCCESS)
		{
			Logger::Log("Could not get number of surface present modes.");
			return false;
		}

		m_currentPhysicalDevicePresentModes.resize(presentModesCount);
		result = vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_presentationSurface, &presentModesCount, &m_currentPhysicalDevicePresentModes[0]);
		if (result != VK_SUCCESS)
		{
			Logger::Log("Could not enumerate surface present modes.");
			return false;
		}

		return true;
	}

	bool Renderer::SelectPresentationMode(VkPresentModeKHR desiredPresentMode)
	{
		for (VkPresentModeKHR presentMode : m_currentPhysicalDevicePresentModes)
		{
			if (presentMode == desiredPresentMode)
			{
				m_presentMode = presentMode;
				return true;
			}
		}

		Logger::Log("Could not select desiredPresentMode, trying to select default FIFO mode instead.");

		for (VkPresentModeKHR presentMode : m_currentPhysicalDevicePresentModes)
		{
			if (presentMode == VK_PRESENT_MODE_FIFO_KHR)
			{
				m_presentMode = VK_PRESENT_MODE_FIFO_KHR;
				return true;
			}
		}
		Logger::Log("Could not select default FIFO presentation mode, is something wrong with the driver?");

		return false;
	}

	bool Renderer::CheckPresentationSurfaceCapabilities(VkPhysicalDevice& device)
	{
		VkResult result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, m_presentationSurface, &m_currentSurfaceCapabilities);
		if (result != VK_SUCCESS)
		{
			Logger::Log("Could not get surface capabilities.");
			return false;
		}

		return true;
	}

	

}