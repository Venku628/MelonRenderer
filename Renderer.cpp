#include "Renderer.h"

namespace MelonRenderer
{

	void MelonRenderer::Renderer::Init(WindowHandle& windowHandle)
	{
		m_windowHandle = windowHandle;

		mat4 projection = glm::perspective(glm::radians(45.0f), 1.0f, 0.1f, 100.0f);
		mat4 view = glm::lookAt(vec3(-5,3,-10),
									vec3(0,0,0),
									vec3(0,-1,0));
		mat4 model = mat4(1.0f);

		// Vulkan clip space has inverted Y and half Z.
		mat4 clip = glm::mat4(1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, -1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 0.5f, 0.0f,
			0.0f, 0.0f, 0.5f, 1.0f);
		m_modelViewProjection = clip*projection*view*model;
		


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

		//TODO: favor dedicated gpu
		//if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)

		// TODO: remember last used device or cycle through devices until one works the first time
		CreateLogicalDeviceAndFollowing(m_physicalDevices[m_currentPhysicalDeviceIndex]);


		Logger::Log("Loading complete.");
	}

	bool Renderer::Tick()
	{
		Draw();

		return false;
	}

	void Renderer::Fini()
	{
		vkDestroySwapchainKHR(m_logicalDevice, m_swapchain, nullptr);
		vkDestroySurfaceKHR(m_vulkanInstance, m_presentationSurface, nullptr);
		vkDestroyDevice(m_logicalDevice, nullptr);
		vkDestroyInstance(m_vulkanInstance, nullptr);

#if defined _WIN32
		FreeLibrary(m_vulkanLibrary);
#elif defined __linux
		dlclose(m_vulkanLibrary);
#endif
		m_vulkanLibrary = nullptr;
	}

	bool Renderer::OnWindowSizeChanged()
	{
		std::cout << "Window resized." << std::endl;
		return false;
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

	void Renderer::CreateLogicalDeviceAndFollowing(VkPhysicalDevice& device)
	{
		CheckRequiredDeviceExtensions(device);
		CheckRequiredDeviceFeatures(device);
		CheckRequiredDeviceProperties(device);
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


		VkResult result = vkCreateDevice(device, &deviceCreateInfo, nullptr, &m_logicalDevice);
		if (result != VK_SUCCESS)
		{
			Logger::Log("Could not create logical device.");
			//TODO: add quick exit functionality here as well
			return ;
		}

		LoadDeviceFunctions();
		LoadDeviceExtensionFunctions();
		AquireQueueHandles();
		CreatePresentationSurface();
		EnumeratePresentationModes(device);
		SelectPresentationMode(VK_PRESENT_MODE_FIFO_KHR); 

		CheckPresentationSurfaceCapabilities(device);
		CreateSwapchain(device);

		//rendering cmd buffer
		CreateCommandBufferPool(m_multipurposeCommandPool, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
		CreateCommandBuffer(m_multipurposeCommandPool, m_multipurposeCommandBuffer);

		//staging cmd buffer pool
		CreateCommandBufferPool(m_stagingBufferCommandPool, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT | VK_COMMAND_POOL_CREATE_TRANSIENT_BIT);

		CreateDepthBuffer();
		CreateUniformBufferMVP();
		CreatePipelineLayout();
		CreateDescriptorPool();
		CreateDescriptorSet();

		CreateRenderPass();
		CreateShaderModules();
		CreateFramebuffers();

		//
		VkVertexInputBindingDescription vertexInputBinding;
		vertexInputBinding.binding = 0;
		vertexInputBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		vertexInputBinding.stride = sizeof(Vertex);
		m_vertexInputBindings.emplace_back(vertexInputBinding);
		//

		//
		VkVertexInputAttributeDescription vertexAttributePosition, vertexAttributeColor;
		vertexAttributePosition.binding = 0;
		vertexAttributePosition.location = 0;
		vertexAttributePosition.format = VK_FORMAT_R32G32B32_SFLOAT;
		vertexAttributePosition.offset = 0;
		m_vertexInputAttributes.emplace_back(vertexAttributePosition);

		vertexAttributeColor.binding = 0;
		vertexAttributeColor.location = 1;
		vertexAttributeColor.format = VK_FORMAT_R32G32B32_SFLOAT;
		vertexAttributeColor.offset = sizeof(float) * 3;
		m_vertexInputAttributes.emplace_back(vertexAttributeColor);
		//

		//---------------------------------------
		m_drawables.resize(4);
		CreateDrawableBuffers(m_drawables[0]);
		CreateDrawableBuffers(m_drawables[1]);
		CreateDrawableBuffers(m_drawables[2]);
		CreateDrawableBuffers(m_drawables[3]);

		m_drawables[0].m_transform = mat4(1.f, 0.f, 0.f, 2.f, 0.f, 1.f, 0.f, 2.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f);
		m_drawables[1].m_transform = mat4(1.f, 0.f, 0.f, -2.f, 0.f, 1.f, 0.f, -2.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f);
		m_drawables[2].m_transform = mat4(1.f, 0.f, 0.f, 2.f, 0.f, 1.f, 0.f, -2.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f);
		m_drawables[3].m_transform = mat4(1.f, 0.f, 0.f, -2.f, 0.f, 1.f, 0.f, 2.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f);
		//---------------------------------------

		CreateGraphicsPipeline();
	}

	bool Renderer::LoadDeviceFunctions()
	{
#define DEVICE_LEVEL_VULKAN_FUNCTION( name ) name = (PFN_##name)vkGetDeviceProcAddr( m_logicalDevice, #name);	\
	if (name == nullptr)																						\
	{std::cout << "Could not load device function named: " #name << std::endl; return false;}					\

#include "loader/ListOfVulkanFunctions.inl"

		return true;
	}

	bool Renderer::LoadDeviceExtensionFunctions()
	{
#define DEVICE_LEVEL_VULKAN_FUNCTION_FROM_EXTENSION( name, extension ) name = (PFN_##name)vkGetDeviceProcAddr( m_logicalDevice, #name);	\
	for(auto & requiredExtension : m_requiredDeviceExtensions){ if(std::string(requiredExtension) == std::string(extension))			\
		{name = (PFN_##name)vkGetDeviceProcAddr( m_logicalDevice, #name);																\
	if (name == nullptr)																												\
	{std::cout << "Could not load device function named: " #name << std::endl; return false;}}}											\

#include "loader/ListOfVulkanFunctions.inl"

		return true;
	}

	bool Renderer::AquireQueueHandles()
	{
		//TODO: define queue requirements and aquire multiple handles accordingly

		vkGetDeviceQueue(m_logicalDevice, 0, 0, &m_multipurposeQueue);


		return true;
	}

	bool Renderer::CreatePresentationSurface()
	{
#ifdef VK_USE_PLATFORM_WIN32_KHR
		VkWin32SurfaceCreateInfoKHR surfaceCreateInfo = {
			VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
			nullptr,
			0,
			m_windowHandle.m_hInstance,
			m_windowHandle.m_HWnd
		};

		VkResult result = vkCreateWin32SurfaceKHR(m_vulkanInstance, &surfaceCreateInfo, nullptr, &m_presentationSurface);

#elif defined VK_USE_PLATFORM_XLIB_KHR
		VkXlibSurfaceCreateInfoKHR surfaceCreateInfo = {
			VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR,
			nullptr,
			0,
			m_windowHandle.m_dpy,
			m_windowHandle.m_window
	};

		VkResult result = vkCreateXlibSurfaceKHR(m_vulkanInstance, &surfaceCreateInfo, nullptr, &m_presentationSurface);

#elif defined VK_USE_PLATFORM_XCB_KHR
		VkXcbSurfaceCreateInfoKHR surfaceCreateInfo = {
			VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR,
			nullptr,
			0,
			m_windowHandle.m_connection,
			m_windowHandle.m_window
		};

		VkResult result = vkCreateXcbSurfaceKHR(m_vulkanInstance, &surfaceCreateInfo, nullptr, &m_presentationSurface);
#endif

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

	bool Renderer::CreateSwapchain(VkPhysicalDevice& device)
	{
		//TODO: make configureable and/or define requirements
		uint32_t desiredNumberOfImages = 2;
		VkExtent2D desiredSizeOfImages = { 640, 480 };
		VkImageUsageFlags desiredImageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		VkSurfaceTransformFlagBitsKHR desiredTransform;

		if (desiredNumberOfImages < m_currentSurfaceCapabilities.minImageCount)
		{
			desiredNumberOfImages = m_currentSurfaceCapabilities.minImageCount;
			Logger::Log("Desired number of images too low, using minimum instead.");
		}

		if (desiredNumberOfImages > m_currentSurfaceCapabilities.maxImageCount)
		{
			desiredNumberOfImages = m_currentSurfaceCapabilities.maxImageCount;
			Logger::Log("Desired number of images too high, using maximum instead.");
		}

		//this signals that the size of the window is determined by the size of the swapchain, we only need the desired size in this case
		if (0xFFFFFFFF == m_currentSurfaceCapabilities.currentExtent.width)
		{
			if (desiredSizeOfImages.width < m_currentSurfaceCapabilities.minImageExtent.width)
			{
				desiredSizeOfImages.width = m_currentSurfaceCapabilities.minImageExtent.width;
				Logger::Log("Desired width of images too low, using minimum instead.");
			}
			else if (desiredSizeOfImages.width > m_currentSurfaceCapabilities.maxImageExtent.width)
			{
				desiredSizeOfImages.width = m_currentSurfaceCapabilities.minImageExtent.width;
				Logger::Log("Desired width of images too high, using maximum instead.");
			}

			if (desiredSizeOfImages.height < m_currentSurfaceCapabilities.minImageExtent.height)
			{
				desiredSizeOfImages.height = m_currentSurfaceCapabilities.minImageExtent.height;
				Logger::Log("Desired height of images too low, using minimum instead.");
			}
			else if (desiredSizeOfImages.height > m_currentSurfaceCapabilities.maxImageExtent.height)
			{
				desiredSizeOfImages.height = m_currentSurfaceCapabilities.minImageExtent.height;
				Logger::Log("Desired height of images too high, using maximum instead.");
			}
		}
		else
		{
			desiredSizeOfImages = m_currentSurfaceCapabilities.currentExtent;
		}

		//TODO: as soon as requirements are defined, implement bitwise checks for both
		desiredImageUsage = m_currentSurfaceCapabilities.supportedUsageFlags;
		desiredTransform = m_currentSurfaceCapabilities.currentTransform;

		uint32_t formatCount = 0;
		VkResult result = vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_presentationSurface, &formatCount, nullptr);
		if (result != VK_SUCCESS)
		{
			Logger::Log("Could not get number of physical device surface formats.");
			return false;
		}

		std::vector<VkSurfaceFormatKHR> surfaceFormats { formatCount };
		result = vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_presentationSurface, &formatCount, &surfaceFormats[0]);
		if (result != VK_SUCCESS)
		{
			Logger::Log("Could not get physical device surface formats.");
			return false;
		}

		//TODO: define format requirement
		VkFormat imageFormat = surfaceFormats[0].format;
		for (auto& surfaceFormat : surfaceFormats)
		{
			if (surfaceFormat.format == VK_FORMAT_R8G8B8A8_UNORM)
			{
				imageFormat = surfaceFormat.format;
				break;
			}
		}
		m_format = imageFormat;
		VkColorSpaceKHR imageColorSpace = surfaceFormats[0].colorSpace;
		for (auto& surfaceFormat : surfaceFormats)
		{
			if (surfaceFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			{
				imageColorSpace = surfaceFormat.colorSpace;
				break;
			}
		}

		//store values for later use
		m_extent.height = desiredSizeOfImages.height;
		m_extent.width = desiredSizeOfImages.width;

		VkSwapchainCreateInfoKHR swapchainCreateInfo = {
			VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
			nullptr,
			0,
			m_presentationSurface,
			desiredNumberOfImages,
			imageFormat,
			imageColorSpace,
			desiredSizeOfImages,
			1,
			desiredImageUsage,
			VK_SHARING_MODE_EXCLUSIVE,
			0,
			nullptr,
			desiredTransform,
			VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
			m_presentMode,
			VK_TRUE,
			VK_NULL_HANDLE //TODO: insert old swapchain if available and destroy after
		};
		swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		swapchainCreateInfo.pNext = nullptr;
		swapchainCreateInfo.surface = m_presentationSurface;
		swapchainCreateInfo.minImageCount = desiredNumberOfImages;
		swapchainCreateInfo.imageFormat = imageFormat;
		swapchainCreateInfo.imageColorSpace = imageColorSpace;
		swapchainCreateInfo.imageExtent = desiredSizeOfImages;
		swapchainCreateInfo.imageArrayLayers = 1;
		swapchainCreateInfo.imageUsage = desiredImageUsage;
		swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		swapchainCreateInfo.queueFamilyIndexCount = 0;
		swapchainCreateInfo.pQueueFamilyIndices = nullptr;
		swapchainCreateInfo.preTransform = desiredTransform;
		swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		swapchainCreateInfo.presentMode = m_presentMode;
		swapchainCreateInfo.clipped = VK_TRUE;
		swapchainCreateInfo.oldSwapchain = VK_NULL_HANDLE; //TODO: insert old swapchain if available and destroy after


		VkBool32 deviceSupported;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, m_queueFamilyIndex, m_presentationSurface, &deviceSupported);
		if (!deviceSupported)
		{
			Logger::Log("Presentation surface not supported by physical device.");
			return false;
		}

		result = vkCreateSwapchainKHR(m_logicalDevice, &swapchainCreateInfo, nullptr, &m_swapchain);
		if (result != VK_SUCCESS)
		{
			Logger::Log("Could not create swapchain.");
			return false;
		}

		// driver may create more images than requested
		uint32_t actualImageCount = 0;
		result = vkGetSwapchainImagesKHR(m_logicalDevice, m_swapchain, &actualImageCount, nullptr);
		if (result != VK_SUCCESS)
		{
			Logger::Log("Could not get number of swapchain images.");
			return false;
		}

		m_swapchainImages.resize(actualImageCount);
		result = vkGetSwapchainImagesKHR(m_logicalDevice, m_swapchain, &actualImageCount, &m_swapchainImages[0]);
		if (result != VK_SUCCESS)
		{
			Logger::Log("Could not enumerate swapchain images.");
			return false;
		}

		m_swapchainImageViews.resize(actualImageCount);
		for (unsigned int i = 0; i < actualImageCount; i++)
		{
			VkImageViewCreateInfo colorImageView = {};
			colorImageView.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			colorImageView.pNext = NULL;
			colorImageView.flags = 0;
			colorImageView.image = m_swapchainImages[i];
			colorImageView.viewType = VK_IMAGE_VIEW_TYPE_2D;
			colorImageView.format = imageFormat;
			colorImageView.components.r = VK_COMPONENT_SWIZZLE_R;
			colorImageView.components.g = VK_COMPONENT_SWIZZLE_G;
			colorImageView.components.b = VK_COMPONENT_SWIZZLE_B;
			colorImageView.components.a = VK_COMPONENT_SWIZZLE_A;
			colorImageView.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			colorImageView.subresourceRange.baseMipLevel = 0;
			colorImageView.subresourceRange.levelCount = 1;
			colorImageView.subresourceRange.baseArrayLayer = 0;
			colorImageView.subresourceRange.layerCount = 1;


			result = vkCreateImageView(m_logicalDevice, &colorImageView, nullptr, &m_swapchainImageViews[i]);
			if (result != VK_SUCCESS)
			{
				Logger::Log("Could not create image view.");
				return false;
			}
		}

		return true;
	}

	bool Renderer::CreateCommandBufferPool(VkCommandPool& commandPool, VkCommandPoolCreateFlags flags)
	{
		VkCommandPoolCreateInfo cmdPoolInfo = {};
		cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		cmdPoolInfo.pNext = NULL;
		cmdPoolInfo.queueFamilyIndex = m_queueFamilyIndex; //TODO: make variable as soon as it is defined as a non constant
		cmdPoolInfo.flags = flags;

		VkResult result = vkCreateCommandPool(m_logicalDevice, &cmdPoolInfo, NULL, &commandPool);
		if (result != VK_SUCCESS)
		{
			Logger::Log("Could not create command pool.");
			return false;
		}

		return true;
	}

	bool Renderer::CreateCommandBuffer(VkCommandPool& commandPool, VkCommandBuffer& commandBuffer)
	{
		VkCommandBufferAllocateInfo cmdInfo = {};
		cmdInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		cmdInfo.pNext = NULL;
		cmdInfo.commandPool = commandPool;
		cmdInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		cmdInfo.commandBufferCount = 1;

		VkResult result = vkAllocateCommandBuffers(m_logicalDevice, &cmdInfo, &commandBuffer);
		if (result != VK_SUCCESS)
		{
			Logger::Log("Could not create command buffer.");
			return false;
		}

		return true;
	}

	bool Renderer::CreateDepthBuffer()
	{
		//find a more elegant solution
		m_extent.depth = 1;
		m_depthBufferFormat = VK_FORMAT_D16_UNORM;

		VkImageCreateInfo imageCreateInfo = {
		VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		nullptr,
		0,
		VK_IMAGE_TYPE_2D,
		VK_FORMAT_D16_UNORM,
		m_extent,
		1, 
		1,
		VK_SAMPLE_COUNT_1_BIT,
		VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
		VK_SHARING_MODE_EXCLUSIVE,
		0, //family queue index count
		nullptr,
		VK_IMAGE_LAYOUT_UNDEFINED
		};

		VkResult result = vkCreateImage(m_logicalDevice, &imageCreateInfo, nullptr, &m_depthBuffer);
		if (result != VK_SUCCESS)
		{
			Logger::Log("Could not create image for depth buffer.");
			return false;
		}

		VkMemoryRequirements memoryReq;
		vkGetImageMemoryRequirements(m_logicalDevice, m_depthBuffer, &memoryReq);
		
		VkMemoryAllocateInfo memoryAllocInfo =
		{
			VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
			nullptr,
			memoryReq.size,
			0
		};
		if (!FindMemoryTypeFromProperties(memoryReq.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &memoryAllocInfo.memoryTypeIndex))
		{
			Logger::Log("Could not find matching memory type from propterties.");
			return false;
		}

		vkAllocateMemory(m_logicalDevice, &memoryAllocInfo, nullptr, &m_depthBufferMemory);
		vkBindImageMemory(m_logicalDevice, m_depthBuffer, m_depthBufferMemory, 0);

		VkImageViewCreateInfo viewInfo = {};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.pNext = nullptr;
		viewInfo.image = m_depthBuffer;
		viewInfo.format = VK_FORMAT_D16_UNORM;
		viewInfo.components.r = VK_COMPONENT_SWIZZLE_R;
		viewInfo.components.g = VK_COMPONENT_SWIZZLE_G;
		viewInfo.components.b = VK_COMPONENT_SWIZZLE_B;
		viewInfo.components.a = VK_COMPONENT_SWIZZLE_A;
		viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.flags = 0;

		result = vkCreateImageView(m_logicalDevice, &viewInfo, nullptr, &m_depthBufferView);
		if (result != VK_SUCCESS)
		{
			Logger::Log("Could not create image view for depth buffer.");
			return false;
		}

		return true;
	}

	//helper function from Vulkan Samples
	bool Renderer::FindMemoryTypeFromProperties(uint32_t typeBits, VkFlags requirements_mask, uint32_t* typeIndex)
	{
		// Search memtypes to find first index with those properties
		for (uint32_t i = 0; i < m_physicalDeviceMemoryProperties.memoryTypeCount; i++) {
			if (typeBits & (1 << i)) {
				// Type is available, does it match user properties?
				if ((m_physicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & requirements_mask) == requirements_mask) {
					*typeIndex = i;
					return true;
				}
			}
		}
		// No memory types matched, return failure
		return false;
	}

	bool Renderer::CreateUniformBufferMVP()
	{
		VkBufferCreateInfo bufferCreateInfo = {
			VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
			nullptr,
			0,
			sizeof(m_modelViewProjection),
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_SHARING_MODE_EXCLUSIVE,
			0, 
			nullptr
		};

		VkResult result = vkCreateBuffer(m_logicalDevice, &bufferCreateInfo, nullptr, &m_uniformBuffer);
		if (result != VK_SUCCESS)
		{
			Logger::Log("Could not create uniform buffer.");
			return false;
		}

		VkMemoryRequirements memoryReq;
		vkGetBufferMemoryRequirements(m_logicalDevice, m_uniformBuffer, &memoryReq);

		VkMemoryAllocateInfo allocateInfo = {
			VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
			nullptr, 
			memoryReq.size,
			0
		};
		if (!FindMemoryTypeFromProperties(memoryReq.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			&allocateInfo.memoryTypeIndex))
		{
			Logger::Log("Could not find memory type from properties for uniform buffer.");
			return false;
		}

		result = vkAllocateMemory(m_logicalDevice, &allocateInfo, nullptr, &m_uniformBufferMemory);
		if (result != VK_SUCCESS)
		{
			Logger::Log("Could not allocate uniform buffer memory.");
			return false;
		}

		uint8_t* pData;
		result = vkMapMemory(m_logicalDevice, m_uniformBufferMemory, 0, memoryReq.size, 0, (void**)& pData);
		if (result != VK_SUCCESS)
		{
			Logger::Log("Could not map memory for uniform buffer memory.");
			return false;
		}
		memcpy(pData, &m_modelViewProjection, sizeof(m_modelViewProjection));
		vkUnmapMemory(m_logicalDevice, m_uniformBufferMemory); //immediatley unmap because of limited page table for gpu+cpu adresses

		result = vkBindBufferMemory(m_logicalDevice, m_uniformBuffer, m_uniformBufferMemory, 0);
		if (result != VK_SUCCESS)
		{
			Logger::Log("Could not bind buffer to memory.");
			return false;
		}

		m_descriptorBufferInfoViewProjection.buffer = m_uniformBuffer;
		m_descriptorBufferInfoViewProjection.offset = 0;
		m_descriptorBufferInfoViewProjection.range = sizeof(m_modelViewProjection);


		return true;
	}

	//simple render pass with color and depth buffer in one subpass
	bool Renderer::CreateRenderPass()
	{
		VkAttachmentDescription attachments[2];
		attachments[0].format = VK_FORMAT_B8G8R8A8_UNORM; // is this correct?
		attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
		attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		attachments[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		attachments[0].flags = 0;

		attachments[1].format = m_depthBufferFormat; 
		attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
		attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		attachments[1].flags = 0;

		VkAttachmentReference colorReference = {};
		colorReference.attachment = 0;
		colorReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentReference depthReference = {};
		depthReference.attachment = 1;
		depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpassDescription = {
			0,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			0,
			nullptr,
			1,
			&colorReference,
			nullptr,
			&depthReference,
			0,
			nullptr
		};

		VkRenderPassCreateInfo renderPassInfo = {
			VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
			nullptr,
			0,
			2,
			attachments,
			1,
			&subpassDescription,
			0,
			nullptr
		};
		VkResult result = vkCreateRenderPass(m_logicalDevice, &renderPassInfo, nullptr, &m_renderPass);
		if (result != VK_SUCCESS)
		{
			Logger::Log("Could not create render pass.");
			return false;
		}


		return true;
	}

	bool Renderer::CreateShaderModule(const std::vector<char>& code, VkShaderModule& shaderModule)
	{
		VkShaderModuleCreateInfo shaderModuleCreateInfo = {
			VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
			nullptr,
			0,
			code.size(),
			reinterpret_cast<const uint32_t*>(code.data())
		};
		VkResult result = vkCreateShaderModule(m_logicalDevice, &shaderModuleCreateInfo, nullptr, &shaderModule);
		if (result != VK_SUCCESS)
		{
			Logger::Log("Could not create shader module.");
			return false;
		}

		return true;
	}

	bool Renderer::CreateShaderModules()
	{
		auto vertShaderCode = readFile("shaders/vert.spv");
		auto fragShaderCode = readFile("shaders/frag.spv");

		VkPipelineShaderStageCreateInfo vertexShader, fragmentShader;

		CreateShaderModule(vertShaderCode, vertexShader.module);
		CreateShaderModule(fragShaderCode, fragmentShader.module);

		vertexShader.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertexShader.pNext = nullptr;
		vertexShader.flags = 0;
		vertexShader.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertexShader.pName = "main";
		vertexShader.pSpecializationInfo = nullptr;
		
		fragmentShader.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragmentShader.pNext = nullptr;
		fragmentShader.flags = 0;
		fragmentShader.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		fragmentShader.pName = "main";
		fragmentShader.pSpecializationInfo = nullptr;
		
		m_shaderStagesV.emplace_back(vertexShader);
		m_shaderStagesV.emplace_back(fragmentShader);

		//TODO: rework function to create all necessary shader modules and handle errors accordingly
		return true;
	}

	bool Renderer::CreateFramebuffers()
	{
		//reuse depthbuffer for all framebuffers
		VkImageView attachments[2];
		attachments[1] = m_depthBufferView;

		VkFramebufferCreateInfo framebufferCreateInfo = {
			VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
			nullptr,
			0,
			m_renderPass,
			2,
			attachments,
			m_extent.width,
			m_extent.height,
			1
		};

		m_framebuffers.resize(m_swapchainImages.size());
		for (int i = 0; i < m_swapchainImages.size(); i++)
		{
			attachments[0] = m_swapchainImageViews[i];
			VkResult result = vkCreateFramebuffer(m_logicalDevice, &framebufferCreateInfo, nullptr, &m_framebuffers[i]);
			if (result != VK_SUCCESS)
			{
				Logger::Log("Could not create framebuffer.");
				return false;
			}
		}

		return true;
	}

	bool Renderer::CreateDrawableBuffers(Drawable& drawable)
	{
		//TODO: make drawable attribute
		uint32_t vertexBufferSize = sizeof(cube_vertex_data);

		VkBuffer vertexStagingBuffer;
		VkDeviceMemory vertexStagingBufferMemory;
		if (!CreateBuffer(vertexBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			vertexStagingBuffer, vertexStagingBufferMemory))
		{
			Logger::Log("Could not create vertex staging buffer.");
			return false;
		}

		void* pVertexData;
		VkResult result = vkMapMemory(m_logicalDevice, vertexStagingBufferMemory, 0, vertexBufferSize, 0, (void**)&pVertexData);
		if (result != VK_SUCCESS)
		{
			Logger::Log("Could not bind vertex staging buffer to memory.");
			return false;
		}
		memcpy(pVertexData, cube_vertex_data, sizeof(cube_vertex_data));
		vkUnmapMemory(m_logicalDevice, vertexStagingBufferMemory);

		if (!CreateBuffer(vertexBufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, 
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			drawable.m_vertexBuffer, drawable.m_vertexBufferMemory))
		{
			Logger::Log("Could not create vertex buffer.");
			return false;
		}

		CopyStagingBuffer(vertexStagingBuffer, drawable.m_vertexBuffer, vertexBufferSize);

		vkDestroyBuffer(m_logicalDevice, vertexStagingBuffer, nullptr);
		vkFreeMemory(m_logicalDevice, vertexStagingBufferMemory, nullptr);

		//index buffer
		//TODO: make drawable attribute
		uint32_t indexBufferSize = sizeof(cube_index_data);
		
		VkBuffer indexStagingBuffer;
		VkDeviceMemory indexStagingBufferMemory;
		if (!CreateBuffer(indexBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			indexStagingBuffer, indexStagingBufferMemory))
		{
			Logger::Log("Could not create index staging buffer.");
			return false;
		}

		void* pIndexData;
		result = vkMapMemory(m_logicalDevice, indexStagingBufferMemory, 0, indexBufferSize, 0, (void**)&pIndexData);
		if (result != VK_SUCCESS)
		{
			Logger::Log("Could not bind index staging buffer to memory.");
			return false;
		}
		memcpy(pIndexData, cube_index_data, sizeof(cube_index_data));
		vkUnmapMemory(m_logicalDevice, indexStagingBufferMemory);

		if (!CreateBuffer(indexBufferSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			drawable.m_indexBuffer, drawable.m_indexBufferMemory))
		{
			Logger::Log("Could not create index buffer.");
			return false;
		}

		CopyStagingBuffer(indexStagingBuffer, drawable.m_indexBuffer, indexBufferSize);

		vkDestroyBuffer(m_logicalDevice, indexStagingBuffer, nullptr);
		vkFreeMemory(m_logicalDevice, indexStagingBufferMemory, nullptr);

		return true;
	}

	bool Renderer::CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
	{
		VkBufferCreateInfo bufferInfo = {};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = size;
		bufferInfo.usage = usage;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VkResult result = vkCreateBuffer(m_logicalDevice, &bufferInfo, nullptr, &buffer);
		if (result != VK_SUCCESS) {
			Logger::Log("Could not create buffer.");
			return false;
		}

		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(m_logicalDevice, buffer, &memRequirements);

		VkMemoryAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		FindMemoryTypeFromProperties(memRequirements.memoryTypeBits, properties, &allocInfo.memoryTypeIndex);

		result = vkAllocateMemory(m_logicalDevice, &allocInfo, nullptr, &bufferMemory);
		if (result != VK_SUCCESS) {
			Logger::Log("Could not allocate buffer memory.");
			return false;
		}

		result = vkBindBufferMemory(m_logicalDevice, buffer, bufferMemory, 0);
		if (result != VK_SUCCESS)
		{
			Logger::Log("Could not bind buffer to memory.");
			return false;
		}

		return true;
	}

	bool Renderer::CreateGraphicsPipeline()
	{
		VkDynamicState dynamicStateEnables[VK_DYNAMIC_STATE_RANGE_SIZE];
		VkPipelineDynamicStateCreateInfo dynamicState = {};
		memset(dynamicStateEnables, 0, sizeof(dynamicStateEnables));
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.pNext = nullptr;
		dynamicState.pDynamicStates = dynamicStateEnables;
		dynamicState.dynamicStateCount = 0;

		VkPipelineVertexInputStateCreateInfo pipelineVertexInputInfo = {};
		pipelineVertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		pipelineVertexInputInfo.pNext = nullptr;
		pipelineVertexInputInfo.flags = 0;
		pipelineVertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(m_vertexInputBindings.size());
		pipelineVertexInputInfo.pVertexBindingDescriptions = m_vertexInputBindings.data();
		pipelineVertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(m_vertexInputAttributes.size());
		pipelineVertexInputInfo.pVertexAttributeDescriptions = m_vertexInputAttributes.data();

		VkPipelineInputAssemblyStateCreateInfo pipelineInputAssemblyInfo = {};
		pipelineInputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		pipelineInputAssemblyInfo.pNext = nullptr;
		pipelineInputAssemblyInfo.flags = 0;
		pipelineInputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		pipelineInputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

		VkPipelineRasterizationStateCreateInfo pipelineRasterizationInfo;
		pipelineRasterizationInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		pipelineRasterizationInfo.pNext = nullptr;
		pipelineRasterizationInfo.flags = 0;
		pipelineRasterizationInfo.polygonMode = VK_POLYGON_MODE_FILL;
		pipelineRasterizationInfo.cullMode = VK_CULL_MODE_BACK_BIT;
		pipelineRasterizationInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
		pipelineRasterizationInfo.depthClampEnable = VK_TRUE;
		pipelineRasterizationInfo.rasterizerDiscardEnable = VK_FALSE;
		pipelineRasterizationInfo.depthBiasEnable = VK_FALSE;
		pipelineRasterizationInfo.depthBiasConstantFactor = 0;
		pipelineRasterizationInfo.depthBiasClamp = 0;
		pipelineRasterizationInfo.depthBiasSlopeFactor = 0;
		pipelineRasterizationInfo.lineWidth = 1.0f;

		VkPipelineColorBlendStateCreateInfo pipelineColorBlendInfo;
		pipelineColorBlendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		pipelineColorBlendInfo.pNext = nullptr;
		pipelineColorBlendInfo.flags = 0;
		VkPipelineColorBlendAttachmentState colorBlendAttachment[1];
		colorBlendAttachment[0].colorWriteMask = 0xf;
		colorBlendAttachment[0].blendEnable = VK_FALSE;
		colorBlendAttachment[0].alphaBlendOp = VK_BLEND_OP_ADD;
		colorBlendAttachment[0].colorBlendOp = VK_BLEND_OP_ADD;
		colorBlendAttachment[0].srcColorBlendFactor = VK_BLEND_FACTOR_ZERO;
		colorBlendAttachment[0].dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
		colorBlendAttachment[0].srcAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		colorBlendAttachment[0].dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		pipelineColorBlendInfo.attachmentCount = 1;
		pipelineColorBlendInfo.pAttachments = colorBlendAttachment;
		pipelineColorBlendInfo.logicOpEnable = VK_FALSE;
		pipelineColorBlendInfo.logicOp = VK_LOGIC_OP_NO_OP;
		pipelineColorBlendInfo.blendConstants[0] = 1.0f;
		pipelineColorBlendInfo.blendConstants[1] = 1.0f;
		pipelineColorBlendInfo.blendConstants[2] = 1.0f;
		pipelineColorBlendInfo.blendConstants[3] = 1.0f;

		VkPipelineViewportStateCreateInfo pipelineViewportInfo = {};
		pipelineViewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		pipelineViewportInfo.pNext = nullptr;
		pipelineViewportInfo.flags = 0;
		pipelineViewportInfo.viewportCount = 1;
		dynamicStateEnables[dynamicState.dynamicStateCount++] = VK_DYNAMIC_STATE_VIEWPORT;
		pipelineViewportInfo.scissorCount = 1;
		dynamicStateEnables[dynamicState.dynamicStateCount++] = VK_DYNAMIC_STATE_SCISSOR;
		pipelineViewportInfo.pScissors = nullptr;
		pipelineViewportInfo.pViewports = nullptr;

		VkPipelineDepthStencilStateCreateInfo pipelineDepthStencilInfo;
		pipelineDepthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		pipelineDepthStencilInfo.pNext = nullptr;
		pipelineDepthStencilInfo.flags = 0;
		pipelineDepthStencilInfo.depthTestEnable = VK_TRUE;
		pipelineDepthStencilInfo.depthWriteEnable = VK_TRUE;
		pipelineDepthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
		pipelineDepthStencilInfo.depthBoundsTestEnable = VK_FALSE;
		pipelineDepthStencilInfo.minDepthBounds = 0;
		pipelineDepthStencilInfo.maxDepthBounds = 0;
		pipelineDepthStencilInfo.stencilTestEnable = VK_FALSE;
		pipelineDepthStencilInfo.back.failOp = VK_STENCIL_OP_KEEP;
		pipelineDepthStencilInfo.back.passOp = VK_STENCIL_OP_KEEP;
		pipelineDepthStencilInfo.back.compareOp = VK_COMPARE_OP_ALWAYS;
		pipelineDepthStencilInfo.back.compareMask = 0;
		pipelineDepthStencilInfo.back.reference = 0;
		pipelineDepthStencilInfo.back.depthFailOp = VK_STENCIL_OP_KEEP;
		pipelineDepthStencilInfo.back.writeMask = 0;
		pipelineDepthStencilInfo.front = pipelineDepthStencilInfo.back;

		//setup for no multisampling for now
		VkPipelineMultisampleStateCreateInfo pipelineMultisampleInfo;
		pipelineMultisampleInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		pipelineMultisampleInfo.pNext = nullptr;
		pipelineMultisampleInfo.flags = 0;
		pipelineMultisampleInfo.pSampleMask = NULL;
		pipelineMultisampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		pipelineMultisampleInfo.sampleShadingEnable = VK_FALSE;
		pipelineMultisampleInfo.alphaToCoverageEnable = VK_FALSE;
		pipelineMultisampleInfo.alphaToOneEnable = VK_FALSE;
		pipelineMultisampleInfo.minSampleShading = 0.0;

		VkGraphicsPipelineCreateInfo pipeline;
		pipeline.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipeline.pNext = nullptr;
		pipeline.layout = m_pipelineLayout;
		pipeline.basePipelineHandle = VK_NULL_HANDLE;
		pipeline.basePipelineIndex = 0;
		pipeline.flags = 0;
		pipeline.pVertexInputState = &pipelineVertexInputInfo;
		pipeline.pInputAssemblyState = &pipelineInputAssemblyInfo;
		pipeline.pRasterizationState = &pipelineRasterizationInfo;
		pipeline.pColorBlendState = &pipelineColorBlendInfo;
		pipeline.pTessellationState = nullptr;
		pipeline.pMultisampleState = &pipelineMultisampleInfo;
		pipeline.pDynamicState = &dynamicState;
		pipeline.pViewportState = &pipelineViewportInfo;
		pipeline.pDepthStencilState = &pipelineDepthStencilInfo;
		pipeline.pStages = m_shaderStagesV.data();
		pipeline.stageCount = static_cast<uint32_t>(m_shaderStagesV.size());
		pipeline.renderPass = m_renderPass;
		pipeline.subpass = 0;

		VkResult result = vkCreateGraphicsPipelines(m_logicalDevice, VK_NULL_HANDLE, 1, &pipeline, nullptr, &m_pipeline);
		if (result != VK_SUCCESS)
		{
			Logger::Log("Could not create graphics pipeline.");
			return false;
		}

		return true;
	}

	bool Renderer::Draw()
	{
		//TODO: make helper function
		VkCommandBufferBeginInfo cmdBufferInfo = {};
		cmdBufferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		cmdBufferInfo.pNext = nullptr;
		cmdBufferInfo.flags = 0;
		cmdBufferInfo.pInheritanceInfo = nullptr;

		VkResult result = vkBeginCommandBuffer(m_multipurposeCommandBuffer, &cmdBufferInfo);
		if (result != VK_SUCCESS)
		{
			Logger::Log("Could not begin command buffer for draw.");
			return false;
		}

		VkClearValue clearValues[2];
		clearValues[0].color.float32[0] = 0.2f;
		clearValues[0].color.float32[1] = 0.2f;
		clearValues[0].color.float32[2] = 0.2f;
		clearValues[0].color.float32[3] = 0.2f;
		clearValues[1].depthStencil.depth = 1.0f;
		clearValues[1].depthStencil.stencil = 0;

		VkSemaphore imageSemaphore;
		VkSemaphoreCreateInfo imageSemaphoreInfo;
		imageSemaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		imageSemaphoreInfo.pNext = nullptr;
		imageSemaphoreInfo.flags = 0;

		result = vkCreateSemaphore(m_logicalDevice, &imageSemaphoreInfo, nullptr, &imageSemaphore);
		if (result != VK_SUCCESS)
		{
			Logger::Log("Could not create draw semaphore.");
			return false;
		}

		result = vkAcquireNextImageKHR(m_logicalDevice, m_swapchain, UINT64_MAX, imageSemaphore, VK_NULL_HANDLE, &m_imageIndex);
		if (result != VK_SUCCESS) //TODO: handle VK_SUBOPTIMAL_KHR and VK_ERROR_OUT_OF_DATE_KHR
		{
			Logger::Log("Could not aquire next image for draw.");
			return false;
		}

		VkRenderPassBeginInfo renderPassBegin;
		renderPassBegin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassBegin.pNext = NULL;
		renderPassBegin.renderPass = m_renderPass;
		renderPassBegin.framebuffer = m_framebuffers[m_imageIndex];
		renderPassBegin.renderArea.offset.x = 0;
		renderPassBegin.renderArea.offset.y = 0;
		renderPassBegin.renderArea.extent.width = m_extent.width;
		renderPassBegin.renderArea.extent.height = m_extent.height;
		renderPassBegin.clearValueCount = 2;
		renderPassBegin.pClearValues = clearValues;

		vkCmdBeginRenderPass(m_multipurposeCommandBuffer , &renderPassBegin, VK_SUBPASS_CONTENTS_INLINE);

		//TODO: scale for multiple objects
		VertexTransform vertexTransformMatrix{ mat4(1.f,0.f,0.f,2.f,
			0.f,1.f,0.f,0.f,
			0.f,0.f,1.f,0.f,
			0.f,0.f,0.f,1.f) };
		VertexTransform vertexTransformMatrix2{ mat4(1.f,0.f,0.f,-2.f,
			0.f,1.f,0.f,0.f,
			0.f,0.f,1.f,0.f,
			0.f,0.f,0.f,1.f) };
		std::vector<VertexTransform> vertexTransforms;
		vertexTransforms.emplace_back(vertexTransformMatrix);
		vertexTransforms.emplace_back(vertexTransformMatrix2);


		vkCmdBindPipeline(m_multipurposeCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);
		std::vector<uint32_t> dynamicOffsets;
		//dynamicOffsets.emplace_back(0);
		vkCmdBindDescriptorSets(m_multipurposeCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineLayout, 0, m_descriptorSets.size(),
			m_descriptorSets.data(), dynamicOffsets.size(), dynamicOffsets.data());

		
		m_viewport.height = (float)m_extent.height;
		m_viewport.width = (float)m_extent.width;
		m_viewport.minDepth = (float)0.0f;
		m_viewport.maxDepth = (float)1.0f;
		m_viewport.x = 0;
		m_viewport.y = 0;
		vkCmdSetViewport(m_multipurposeCommandBuffer, 0, 1, &m_viewport);

		m_scissorRect2D.extent.width = m_extent.width;
		m_scissorRect2D.extent.height = m_extent.height;
		m_scissorRect2D.offset.x = 0;
		m_scissorRect2D.offset.y = 0;
		vkCmdSetScissor(m_multipurposeCommandBuffer, 0, 1, &m_scissorRect2D);

		/*
		const VkDeviceSize offsets[1] = { 0 };
		vkCmdBindVertexBuffers(m_multipurposeCommandBuffer, 0, 1, &m_vertexBuffer, offsets);
		vkCmdBindIndexBuffer(m_multipurposeCommandBuffer, m_indexBuffer, 0, VK_INDEX_TYPE_UINT32);


		vkCmdPushConstants(m_multipurposeCommandBuffer, m_pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, 
			sizeof(vertexTransformMatrix), &vertexTransformMatrix);
		vkCmdDrawIndexed(m_multipurposeCommandBuffer, sizeof(cube_index_data)/sizeof(uint32_t), 1, 0, 0, 0);
		vkCmdPushConstants(m_multipurposeCommandBuffer, m_pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, 
			sizeof(vertexTransformMatrix2), &vertexTransformMatrix2);
		vkCmdDrawIndexed(m_multipurposeCommandBuffer, sizeof(cube_index_data)/sizeof(uint32_t), 1, 0, 0, 0);
		*/

		//------------------------------------
		for (auto& drawable : m_drawables)
		{
			drawable.Tick(m_multipurposeCommandBuffer, m_pipelineLayout);
		}
		//------------------------------------

		vkCmdEndRenderPass(m_multipurposeCommandBuffer);
		result = vkEndCommandBuffer(m_multipurposeCommandBuffer);
		if (result != VK_SUCCESS) 
		{
			Logger::Log("Could not end command buffer for draw.");
			return false;
		}

		//TODO: change this 
		const VkCommandBuffer cmd[] = { m_multipurposeCommandBuffer };
		VkFenceCreateInfo fenceInfo;
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.pNext = nullptr;
		fenceInfo.flags = 0;
		VkFence drawFence;
		vkCreateFence(m_logicalDevice, &fenceInfo, nullptr, &drawFence);

		VkPipelineStageFlags pipelineStageFlags = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		VkSubmitInfo submitInfo[1] = {};
		submitInfo[0].pNext = nullptr;
		submitInfo[0].sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo[0].waitSemaphoreCount = 1;
		submitInfo[0].pWaitSemaphores = &imageSemaphore;
		submitInfo[0].pWaitDstStageMask = &pipelineStageFlags;
		submitInfo[0].commandBufferCount = 1;
		submitInfo[0].pCommandBuffers = cmd;
		submitInfo[0].signalSemaphoreCount = 0;
		submitInfo[0].pSignalSemaphores = nullptr;
		result = vkQueueSubmit(m_multipurposeQueue, 1, submitInfo, drawFence);
		if (result != VK_SUCCESS)
		{
			Logger::Log("Could not submit draw queue.");
			return false;
		}

		VkPresentInfoKHR presentInfo;
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.pNext = nullptr;
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = &m_swapchain;
		presentInfo.pImageIndices = &m_imageIndex;
		presentInfo.pWaitSemaphores = nullptr;
		presentInfo.waitSemaphoreCount = 0;
		presentInfo.pResults = nullptr;

		//wait for buffer to be finished
		do {
			result = vkWaitForFences(m_logicalDevice, 1, &drawFence, VK_TRUE, 10000000);
		} while (result == VK_TIMEOUT);
		if (result != VK_SUCCESS)
		{
			Logger::Log("Could not wait for draw fences.");
			return false;
		}

		result = vkQueuePresentKHR(m_multipurposeQueue, &presentInfo);
		if (result != VK_SUCCESS)
		{
			Logger::Log("Could not present the draw queue.");
			return false;
		}

		return true;
	}

	bool Renderer::CreatePipelineLayout()
	{
		//TODO: split and scale
		std::vector<VkDescriptorSetLayoutBinding> layoutBindings;
		unsigned int layoutBindingIndex = 0;

		VkDescriptorSetLayoutBinding layoutBindingViewProjection = {
			layoutBindingIndex++, //only one descriptor set for now
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			1,
			VK_SHADER_STAGE_VERTEX_BIT, // = bound to vertex shader
			nullptr
		};
		layoutBindings.emplace_back(layoutBindingViewProjection);
		/*
		VkDescriptorSetLayoutBinding layoutBindingModel = {
			layoutBindingIndex++, //only one descriptor set for now
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
			1,
			VK_SHADER_STAGE_VERTEX_BIT, // = bound to vertex shader
			nullptr
		};
		layoutBindings.emplace_back(layoutBindingModel);
		*/
		

		VkDescriptorSetLayoutCreateInfo descriptorLayout = {
			VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
			nullptr,
			0,
			layoutBindings.size(),
			layoutBindings.data() //only single element for now
		};
		m_descriptorSetLayouts.resize(1);
		VkResult result = vkCreateDescriptorSetLayout(m_logicalDevice, &descriptorLayout, nullptr, &m_descriptorSetLayouts[0]);
		if (result != VK_SUCCESS)
		{
			Logger::Log("Could not create uniform buffer descriptor set layout.");
			return false;
		}

		std::vector<VkPushConstantRange> pushConstantRanges;
		VkPushConstantRange pushConstantRangeTransforms = {};
		pushConstantRangeTransforms.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		pushConstantRangeTransforms.offset = 0;
		pushConstantRangeTransforms.size = sizeof(VertexTransform);
		pushConstantRanges.emplace_back(pushConstantRangeTransforms);
		
		VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {
			VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
			nullptr,
			0,
			m_descriptorSetLayouts.size(),
			m_descriptorSetLayouts.data(),
			pushConstantRanges.size(),
			pushConstantRanges.data()
		};
		result = vkCreatePipelineLayout(m_logicalDevice, &pipelineLayoutCreateInfo, nullptr, &m_pipelineLayout);
		if (result != VK_SUCCESS)
		{
			Logger::Log("Could not create pipeline layout.");
			return false;
		}

		return true;
	}

	bool Renderer::CreateDescriptorPool()
	{
		std::vector<VkDescriptorPoolSize> descriptorPoolSizes;
		VkDescriptorPoolSize poolSizeViewProjection = {};
		poolSizeViewProjection.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSizeViewProjection.descriptorCount = 1;
		descriptorPoolSizes.emplace_back(poolSizeViewProjection);
		/*
		VkDescriptorPoolSize poolSizeModel = {};
		poolSizeModel.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
		poolSizeModel.descriptorCount = 1;
		descriptorPoolSizes.emplace_back(poolSizeModel);
		*/
		
		VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = {
			VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
			nullptr,
			0,
			1, //TODO: max sets, make non constant, take number from CreatePipelineLayout
			descriptorPoolSizes.size(),
			descriptorPoolSizes.data()
		};
		VkResult result = vkCreateDescriptorPool(m_logicalDevice, &descriptorPoolCreateInfo, nullptr, &m_descriptorPool);
		if (result != VK_SUCCESS)
		{
			Logger::Log("Could not create descriptor pool.");
			return false;
		}

		return true;
	}

	bool Renderer::CreateDescriptorSet()
	{
		VkDescriptorSetAllocateInfo allocInfo;
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.pNext = nullptr;
		allocInfo.descriptorPool = m_descriptorPool;
		allocInfo.descriptorSetCount = m_descriptorSetLayouts.size(); //m_descriptorSetLayouts.size()
		allocInfo.pSetLayouts = m_descriptorSetLayouts.data();
		m_descriptorSets.resize(1);
		VkResult result = vkAllocateDescriptorSets(m_logicalDevice, &allocInfo, m_descriptorSets.data());
		if (result != VK_SUCCESS)
		{
			Logger::Log("Could not allocate descriptor set.");
			return false;
		}

		VkWriteDescriptorSet writes[1];
		writes[0] = {};
		writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writes[0].pNext = nullptr;
		writes[0].dstSet = m_descriptorSets[0];
		writes[0].descriptorCount = 1;
		writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		writes[0].pBufferInfo = &m_descriptorBufferInfoViewProjection;
		writes[0].dstArrayElement = 0;
		writes[0].dstBinding = 0;
		/*
		writes[1] = {};
		writes[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writes[1].pNext = nullptr;
		writes[1].dstSet = m_descriptorSets[0];
		writes[1].descriptorCount = 1;
		writes[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
		writes[1].pBufferInfo = &m_descriptorBufferInfoViewProjection; //TODO: replace with other uniform buffer
		writes[1].dstArrayElement = 0;
		writes[1].dstBinding = 1;
		*/
		vkUpdateDescriptorSets(m_logicalDevice, 1, writes, 0, nullptr);

		return true;
	}

	bool Renderer::CopyStagingBuffer(VkBuffer cpuVisibleBuffer, VkBuffer gpuOnlyBuffer, VkDeviceSize size)
	{
		VkCommandBufferAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.pNext = nullptr;
		allocInfo.commandPool = m_stagingBufferCommandPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = 1;

		VkCommandBuffer copyCommandBuffer;
		VkResult result = vkAllocateCommandBuffers(m_logicalDevice, &allocInfo, &copyCommandBuffer);
		if (result != VK_SUCCESS)
		{
			Logger::Log("Could not allocate command buffer for copying staging buffer.");
			return false;
		}

		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.pNext = nullptr;
		beginInfo.flags = 0;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		result = vkBeginCommandBuffer(copyCommandBuffer, &beginInfo);
		if (result != VK_SUCCESS)
		{
			Logger::Log("Could not begin command buffer for copying staging buffer.");
			return false;
		}

		VkBufferCopy copyRegion = {};
		copyRegion.srcOffset = 0; // Optional
		copyRegion.dstOffset = 0; // Optional
		copyRegion.size = size;
		vkCmdCopyBuffer(copyCommandBuffer, cpuVisibleBuffer, gpuOnlyBuffer, 1, &copyRegion);

		result = vkEndCommandBuffer(copyCommandBuffer);
		if (result != VK_SUCCESS)
		{
			Logger::Log("Could not end command buffer for copying staging buffer.");
			return false;
		}

		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.pNext = nullptr;
		submitInfo.waitSemaphoreCount = 0;
		submitInfo.pWaitSemaphores = nullptr;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &copyCommandBuffer;
		submitInfo.signalSemaphoreCount = 0;
		submitInfo.pSignalSemaphores = nullptr;

		result = vkQueueSubmit(m_multipurposeQueue, 1, &submitInfo, VK_NULL_HANDLE);
		if (result != VK_SUCCESS)
		{
			Logger::Log("Could not submit queue for copying staging buffer.");
			return false;
		}
		result = vkQueueWaitIdle(m_multipurposeQueue);
		if (result != VK_SUCCESS)
		{
			Logger::Log("Could not wait for idle queue for copying staging buffer.");
			return false;
		}

		vkFreeCommandBuffers(m_logicalDevice, m_stagingBufferCommandPool, 1, &copyCommandBuffer);

		return true;
	}

	bool Renderer::AquireNextImage()
	{
		VkResult result = vkAcquireNextImageKHR(m_logicalDevice, m_swapchain, 2000000000, m_semaphore, m_fence, &m_currentImageIndex);
		if ((result != VK_SUCCESS) && (result != VK_SUBOPTIMAL_KHR))
		{
			Logger::Log("Could not aquire next image.");
			return false;
		}

		//TODO: if VK_ERROR_OUT_OF_DATE_KHR, swapchain has to be recreated

		return true;
	}

	bool Renderer::PresentImage()
	{
		//TODO: adjust to multiple swapchains etc
		VkPresentInfoKHR presentInfo = {
			VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
			nullptr,
			1, //semaphore array size
			&m_semaphore,
			1, //swapchain array size
			&m_swapchain,
			&m_currentImageIndex,
			nullptr
		};

		VkResult result = vkQueuePresentKHR(m_multipurposeQueue, &presentInfo);
		if (result != VK_SUCCESS)
		{
			Logger::Log("Could not present image");
			return false;
		}

		return true;
	}

}