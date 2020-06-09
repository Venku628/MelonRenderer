#include "Renderer.h"

namespace MelonRenderer
{

	void MelonRenderer::Renderer::Init()
	{
		CreateGLFWWindow();

		timeLast = timeNow = std::chrono::high_resolution_clock::now();

		LoadVulkanLibrary();
		LoadExportedFunctions();
		LoadGlobalFunctions();
		CheckInstanceExtensions();
		CreateInstance();
		LoadInstanceFunctions();
		LoadInstanceExtensionFunctions();
		EnumeratePhysicalDevices();

		m_requiredDeviceExtensions.emplace_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
		m_requiredDeviceExtensions.emplace_back(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
		
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

		m_memoryManager.Init(m_physicalDeviceMemoryProperties, m_currentPhysicalDeviceProperties);

		OutputSurface outputSurface;
		outputSurface.capabilites = m_currentSurfaceCapabilities;
		outputSurface.surface = m_presentationSurface;

		ImGui::CreateContext();
		ImGui::StyleColorsDark();
		ImGui::GetIO().BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;
		ImGui::GetIO().DisplaySize.x = defaultWidth;
		ImGui::GetIO().DisplaySize.y = defaultHeight;
		GlfwInputInit();

		CreateRenderpass();

		//TODO: move to simple scene graph, when a camera node is constructed
		m_camera.Init(m_memoryManager);
		/*
		m_rasterizationPipeline.SetCamera(&m_camera);
		m_rasterizationPipeline.SetScene(&m_scene);

		m_rasterizationPipeline.Init(m_physicalDevices[m_currentPhysicalDeviceIndex], m_memoryManager, m_renderpass, m_extent);
		m_rasterizationPipeline.FillAttachments(m_swapchain.GetAttachmentPointer());
		*/
		m_imguiPipeline.Init(m_physicalDevices[m_currentPhysicalDeviceIndex], m_memoryManager, m_renderpass, m_extent);
		m_swapchain.CreateSwapchain(m_physicalDevices[m_currentPhysicalDeviceIndex], &m_renderpass, outputSurface, m_extent);


		//-----------------------------------------
		Drawable cube, dragon, mirror, bunny, teapot, scene;
		cube.Init(m_memoryManager);
		m_scene.m_drawables.emplace_back(cube);
		dragon.Init(m_memoryManager, "models/dragon.obj");
		m_scene.m_drawables.emplace_back(dragon);
		mirror.Init(m_memoryManager, "models/mirror.obj");
		m_scene.m_drawables.emplace_back(mirror);
		bunny.Init(m_memoryManager, "models/bunny.obj");
		m_scene.m_drawables.emplace_back(bunny);
		teapot.Init(m_memoryManager, "models/teapot.obj");
		m_scene.m_drawables.emplace_back(teapot); //test if unused geometry causes problems
		scene.Init(m_memoryManager, "models/scene.obj");
		m_scene.m_drawables.emplace_back(scene);

		// random order of models to test correct uploading
		m_drawableNodes.resize(7);
		//scene
		*m_drawableNodes[0].GetTransformMat() = glm::scale(mat4(1.f), vec3(50.f, 50.f, 50.f));
		m_drawableNodes[0].SetDrawableInstance(m_scene.CreateDrawableInstance(5, false));
		m_scene.m_rootChildren.emplace_back(&m_drawableNodes[0]);
		//mirror 1
		*m_drawableNodes[1].GetTransformMat() = mat4(1.f, 0.f, 0.f, -40.f, 0.f, 1.f, 0.f, 20.f, 0.f, 0.f, 1.f, -80.f, 0.f, 0.f, 0.f, 1.f);
		m_drawableNodes[1].SetDrawableInstance(m_scene.CreateDrawableInstance(2, false));
		m_scene.m_rootChildren.emplace_back(&m_drawableNodes[1]);
		//mirror 2 
		*m_drawableNodes[2].GetTransformMat() = mat4(1.f, 0.f, 0.f, 40.f, 0.f, 1.f, 0.f, 20.f, 0.f, 0.f, 1.f, -51.f, 0.f, 0.f, 0.f, 1.f);
		*m_drawableNodes[2].GetTransformMat() = glm::rotate(*m_drawableNodes[2].GetTransformMat(), glm::radians(180.f), vec3(0.f, 1.f, 0.f));
		m_drawableNodes[2].SetDrawableInstance(m_scene.CreateDrawableInstance(2, false));
		m_scene.m_rootChildren.emplace_back(&m_drawableNodes[2]);
		//object node
		*m_objectNode.GetTransformMat() = mat4(10.f, 0.f, 0.f, -50.f, 0.f, 10.f, 0.f, 20.f, 0.f, 0.f, 10.f, -50.f, 0.f, 0.f, 0.f, 1.f);

		//cube
		*m_drawableNodes[3].GetTransformMat() = mat4(0.1f, 0.f, 0.f, -2.f, 0.f, 0.1f, 0.f, 2.f, 0.f, 0.f, 0.1f, -2.f, 0.f, 0.f, 0.f, 1.f);
		m_drawableNodes[3].SetDrawableInstance(m_scene.CreateDrawableInstance(0, false));
		m_objectNode.m_children.emplace_back(&m_drawableNodes[3]);
		//bunny
		*m_drawableNodes[4].GetTransformMat() = mat4(2.f, 0.f, 0.f, -2.f, 0.f, 2.f, 0.f, -2.f, 0.f, 0.f, 2.f, 0.f, 0.f, 0.f, 0.f, 1.f);
		m_drawableNodes[4].SetDrawableInstance(m_scene.CreateDrawableInstance(3, false));
		m_objectNode.m_children.emplace_back(&m_drawableNodes[4]);
		//dragon 1
		*m_drawableNodes[5].GetTransformMat() = mat4(1.f, 0.f, 0.f, -2.f, 0.f, 1.f, 0.f, 2.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f);
		m_drawableNodes[5].SetDrawableInstance(m_scene.CreateDrawableInstance(1, false));
		m_objectNode.m_children.emplace_back(&m_drawableNodes[5]);
		// dragon 2
		*m_drawableNodes[6].GetTransformMat() = mat4(1.f); 
		m_drawableNodes[6].SetDrawableInstance(m_scene.CreateDrawableInstance(1, false));
		m_objectNode.m_children.emplace_back(&m_drawableNodes[6]);
		
		m_scene.m_rootChildren.emplace_back(&m_objectNode);
		
		m_scene.UpdateInstanceTransforms();
		//-----------------------------------------

		m_raytracingPipeline.SetRaytracingProperties(&m_raytracingProperties);
		m_raytracingPipeline.SetScene(&m_scene);
		m_raytracingPipeline.SetCamera(&m_camera);
		m_raytracingPipeline.Init(m_physicalDevices[m_currentPhysicalDeviceIndex], m_memoryManager, m_renderpass, m_extent);
		
		Logger::Log("Loading complete.");
	}

	bool Renderer::Tick()
	{
		timeNow = std::chrono::high_resolution_clock::now();
		float timeDelta = static_cast<float>(std::chrono::duration_cast<std::chrono::nanoseconds>(timeNow - timeLast).count());
		float fps = 1000000000.f / timeDelta;
		std::string logMessage = "FPS: ";
		//Logger::Log(logMessage.append(std::to_string(fps)));
		timeLast = timeNow;

		constexpr auto FPS_AVERAGE_RANGE = 500;

		static float frameTimes[FPS_AVERAGE_RANGE];
		static uint32_t frameIndex = 0;
		frameTimes[frameIndex] = fps;

		float fpsAverage = 0;
		for (int i = 0; i < FPS_AVERAGE_RANGE; i++)
		{
			fpsAverage += frameTimes[i];
		}
		fpsAverage /= FPS_AVERAGE_RANGE;

		ImGui::Begin("FPS Counter");
		ImGui::Text(std::to_string(fpsAverage).c_str());
		ImGui::End();

		frameIndex++;
		if (frameIndex == FPS_AVERAGE_RANGE)
			frameIndex = 0;
		//--------------------------------------------------------------------


		m_swapchain.AquireNextImage();
		VkCommandBuffer& commandBuffer = m_swapchain.GetCommandBuffer();
		BeginCommandBuffer(commandBuffer);

		m_camera.Tick(m_window);




		//TODO: switch between raytracing and rasterization
		m_raytracingPipeline.Tick(commandBuffer);
		//m_rasterizationPipeline.Tick(commandBuffer);

		CopyOutputToSwapchain(commandBuffer, m_raytracingPipeline.GetStorageImage());
		
		ImGui::Render();
		BeginRenderpass(commandBuffer);
		m_imguiPipeline.Tick(commandBuffer);
		EndRenderpass(commandBuffer);
		
		EndCommandBuffer(commandBuffer);
		
		m_swapchain.PresentImage();

		return true;
	}

	void Renderer::Loop()
	{
		while (!glfwWindowShouldClose(m_window))
		{
			Logger::Get().Print();

			glfwPollEvents();

			//TODO: use proper callbacks
			int glfwWidth, glfwHeight;
			glfwGetWindowSize(m_window, &glfwWidth, &glfwHeight);
			if ((glfwWidth != m_extent.width) || (glfwHeight != m_extent.height))
			{
				m_extent.width = glfwWidth;
				m_extent.height = glfwHeight;

				ImGui::GetIO().DisplaySize.x = glfwWidth;
				ImGui::GetIO().DisplaySize.y = glfwHeight;

				Resize();
			}
			
			GlfwInputTick();
			ImGui::NewFrame();
			Tick();
		}
	}

	void Renderer::Fini()
	{
		ImGui::DestroyContext();
		
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
		m_window = glfwCreateWindow(defaultWidth, defaultHeight, "Vulkan Renderer", nullptr, nullptr);
		m_extent.width = defaultWidth;
		m_extent.height = defaultHeight;

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
			//"VK_LAYER_KHRONOS_validation",
			"VK_LAYER_LUNARG_standard_validation"
		};
		instanceCreateInfo.enabledLayerCount = static_cast<unsigned int>(validationLayers.size());
		instanceCreateInfo.ppEnabledLayerNames = validationLayers.data();
		Logger::Log("Activated validation layers for debugging.");
#else
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

		//check raytracing capabilities at this point because it�s convenient
		m_hasRaytracingCapabilities = false;
		for (auto& deviceExtension : deviceExtensions)
		{
			if (!strcmp(deviceExtension.extensionName, VK_NV_RAY_TRACING_EXTENSION_NAME))
			{
				Logger::Log("Device supports raytracing!");
				m_hasRaytracingCapabilities = true;
				break;
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

		m_raytracingProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PROPERTIES_NV;
		m_currentPhysicalDeviceProperties2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
		m_currentPhysicalDeviceProperties2.pNext = &m_raytracingProperties;

		vkGetPhysicalDeviceProperties2(device, &m_currentPhysicalDeviceProperties2);

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

		auto deviceExtensionsToActivate = m_requiredDeviceExtensions;
		/*
		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions;
		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
		deviceExtensionsToActivate.emplace_back(&glfwExtensions);
		*/

		if (m_hasRaytracingCapabilities)
		{
			deviceExtensionsToActivate.emplace_back("VK_KHR_get_memory_requirements2");
			deviceExtensionsToActivate.emplace_back("VK_NV_ray_tracing");
			deviceExtensionsToActivate.emplace_back(VK_KHR_MAINTENANCE3_EXTENSION_NAME);
		}


		VkPhysicalDeviceDescriptorIndexingFeaturesEXT indexingFeatures{};
		indexingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES_EXT;
		indexingFeatures.pNext = nullptr;

		VkPhysicalDeviceFeatures2 features2 = {};
		features2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
		features2.pNext = &indexingFeatures;
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
		deviceCreateInfo.pEnabledFeatures = nullptr; 

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

		return true;
	}

	bool Renderer::CreateRenderpass()
	{ 
		VkAttachmentDescription colorAttachment = {};
		colorAttachment.format = VK_FORMAT_B8G8R8A8_UNORM; //TODO: parameter
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD; //previously clear, would overwrite render image
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentReference colorAttachmentReference = {};
		colorAttachmentReference.attachment = 0;
		colorAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		//TODO: add depth attachment and reference when using rasterisation

		VkSubpassDescription subpasses[1];
		subpasses[0] = {};
		subpasses[0].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpasses[0].colorAttachmentCount = 1;
		subpasses[0].pColorAttachments = &colorAttachmentReference;
		subpasses[0].pDepthStencilAttachment = nullptr;

		VkSubpassDependency dependencies[1];

		dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
		dependencies[0].dstSubpass = 0;
		dependencies[0].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependencies[0].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		dependencies[0].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

		VkAttachmentDescription attachments[1] = { colorAttachment };
		VkRenderPassCreateInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = 1;
		renderPassInfo.pAttachments = attachments;
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = subpasses;
		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = dependencies;
		VkResult result = vkCreateRenderPass(Device::Get().m_device, &renderPassInfo, nullptr, &m_renderpass);
		if (result != VK_SUCCESS)
		{
			Logger::Log("Could not create renderpass.");
			return false;
		}

		return true;
	}

	bool Renderer::BeginRenderpass(VkCommandBuffer& commandBuffer)
	{
		VkRenderPassBeginInfo renderPassBegin = {};
		renderPassBegin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassBegin.pNext = nullptr;
		renderPassBegin.renderPass = m_renderpass;
		renderPassBegin.framebuffer = m_swapchain.GetFramebuffer();
		renderPassBegin.renderArea.offset.x = 0;
		renderPassBegin.renderArea.offset.y = 0;
		renderPassBegin.renderArea.extent.width = m_extent.width;
		renderPassBegin.renderArea.extent.height = m_extent.height;
		renderPassBegin.clearValueCount = 0;
		renderPassBegin.pClearValues = nullptr;
		vkCmdBeginRenderPass(commandBuffer, &renderPassBegin, VK_SUBPASS_CONTENTS_INLINE);

		return true;
	}

	bool Renderer::EndRenderpass(VkCommandBuffer& commandBuffer)
	{
		vkCmdEndRenderPass(commandBuffer);

		return true;
	}

	bool Renderer::BeginCommandBuffer(VkCommandBuffer& commandBuffer)
	{
		VkCommandBufferBeginInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		VkResult result = vkBeginCommandBuffer(commandBuffer, &info);
		if (result != VK_SUCCESS)
		{
			Logger::Log("Could not begin command buffer for render pass.");
			return false;
		}

		return true;
	}

	bool Renderer::EndCommandBuffer(VkCommandBuffer& commandBuffer)
	{
		VkResult result = vkEndCommandBuffer(commandBuffer);
		if (result != VK_SUCCESS)
		{
			Logger::Log("Could not end command buffer for render pass.");
			return false;
		}

		return true;
	}

	bool Renderer::CopyOutputToSwapchain(VkCommandBuffer& commandBuffer, VkImage storage)
	{
		m_memoryManager.TransitionImageLayout(commandBuffer, m_swapchain.GetImage(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		m_memoryManager.TransitionImageLayout(commandBuffer, storage, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

		VkImageCopy copyRegion{};
		copyRegion.srcSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
		copyRegion.srcOffset = { 0, 0, 0 };
		copyRegion.dstSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
		copyRegion.dstOffset = { 0, 0, 0 };
		copyRegion.extent = { m_extent.width, m_extent.height, 1 };
		vkCmdCopyImage(commandBuffer, storage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, m_swapchain.GetImage(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);

		m_memoryManager.TransitionImageLayout(commandBuffer, m_swapchain.GetImage(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
		m_memoryManager.TransitionImageLayout(commandBuffer, storage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_GENERAL);

		return true;
	}

	bool Renderer::Resize()
	{
		OutputSurface outputSurface;
		CheckPresentationSurfaceCapabilities(m_physicalDevices[m_currentPhysicalDeviceIndex]);
		outputSurface.capabilites = m_currentSurfaceCapabilities;
		outputSurface.surface = m_presentationSurface;

		m_imguiPipeline.RecreateOutput(m_extent);
		//m_rasterizationPipeline.RecreateOutput(m_extent);
		m_raytracingPipeline.RecreateOutput(m_extent);
		m_swapchain.CleanupSwapchain(true);
		//m_rasterizationPipeline.FillAttachments(m_swapchain.GetAttachmentPointer());
		m_swapchain.CreateSwapchain(m_physicalDevices[m_currentPhysicalDeviceIndex], &m_renderpass, outputSurface, m_extent);

		return true;
	}

	bool Renderer::GlfwInputInit()
	{
		// Setup back-end capabilities flags
		ImGuiIO& io = ImGui::GetIO();
		io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;  
		io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;        
		io.BackendPlatformName = "MelonRenderer";

		// Keyboard mapping. ImGui will use those indices to peek into the io.KeysDown[] array.
		io.KeyMap[ImGuiKey_Tab] = GLFW_KEY_TAB;
		io.KeyMap[ImGuiKey_LeftArrow] = GLFW_KEY_LEFT;
		io.KeyMap[ImGuiKey_RightArrow] = GLFW_KEY_RIGHT;
		io.KeyMap[ImGuiKey_UpArrow] = GLFW_KEY_UP;
		io.KeyMap[ImGuiKey_DownArrow] = GLFW_KEY_DOWN;
		io.KeyMap[ImGuiKey_PageUp] = GLFW_KEY_PAGE_UP;
		io.KeyMap[ImGuiKey_PageDown] = GLFW_KEY_PAGE_DOWN;
		io.KeyMap[ImGuiKey_Home] = GLFW_KEY_HOME;
		io.KeyMap[ImGuiKey_End] = GLFW_KEY_END;
		io.KeyMap[ImGuiKey_Insert] = GLFW_KEY_INSERT;
		io.KeyMap[ImGuiKey_Delete] = GLFW_KEY_DELETE;
		io.KeyMap[ImGuiKey_Backspace] = GLFW_KEY_BACKSPACE;
		io.KeyMap[ImGuiKey_Space] = GLFW_KEY_SPACE;
		io.KeyMap[ImGuiKey_Enter] = GLFW_KEY_ENTER;
		io.KeyMap[ImGuiKey_Escape] = GLFW_KEY_ESCAPE;
		io.KeyMap[ImGuiKey_KeyPadEnter] = GLFW_KEY_KP_ENTER;
		io.KeyMap[ImGuiKey_A] = GLFW_KEY_A;
		io.KeyMap[ImGuiKey_C] = GLFW_KEY_C;
		io.KeyMap[ImGuiKey_V] = GLFW_KEY_V;
		io.KeyMap[ImGuiKey_X] = GLFW_KEY_X;
		io.KeyMap[ImGuiKey_Y] = GLFW_KEY_Y;
		io.KeyMap[ImGuiKey_Z] = GLFW_KEY_Z;

		io.SetClipboardTextFn = SetClipboardText;
		io.GetClipboardTextFn = GetClipboardText;
		io.ClipboardUserData = m_window;
		
		m_inputData.m_mouseCursors[ImGuiMouseCursor_Arrow] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
		m_inputData.m_mouseCursors[ImGuiMouseCursor_TextInput] = glfwCreateStandardCursor(GLFW_IBEAM_CURSOR);
		m_inputData.m_mouseCursors[ImGuiMouseCursor_ResizeNS] = glfwCreateStandardCursor(GLFW_VRESIZE_CURSOR);
		m_inputData.m_mouseCursors[ImGuiMouseCursor_ResizeEW] = glfwCreateStandardCursor(GLFW_HRESIZE_CURSOR);
		m_inputData.m_mouseCursors[ImGuiMouseCursor_Hand] = glfwCreateStandardCursor(GLFW_HAND_CURSOR);

		m_inputData.m_mouseCursors[ImGuiMouseCursor_ResizeAll] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
		m_inputData.m_mouseCursors[ImGuiMouseCursor_ResizeNESW] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
		m_inputData.m_mouseCursors[ImGuiMouseCursor_ResizeNWSE] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
		m_inputData.m_mouseCursors[ImGuiMouseCursor_NotAllowed] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);

		glfwSetWindowUserPointer(m_window, reinterpret_cast<void*>(&m_inputData));

		m_inputData.m_prevUserCallbackMousebutton = glfwSetMouseButtonCallback(m_window, MouseButtonCallback);
		m_inputData.m_prevUserCallbackScroll = glfwSetScrollCallback(m_window, ScrollCallback);
		m_inputData.m_prevUserCallbackKey = glfwSetKeyCallback(m_window, KeyCallback);
		m_inputData.m_prevUserCallbackChar = glfwSetCharCallback(m_window, CharCallback);

		return true;
	}

	bool Renderer::GlfwInputTick()
	{
		if (!GlfwUpdateMousePosAndButtons())
			return false;
		if (!GlfwUpdateMouseCursor())
			return false;
		if (!GlfwUpdateGamepads())
			return false;

		return true;
	}

	bool Renderer::GlfwUpdateMousePosAndButtons()
	{
		// Update buttons
		ImGuiIO& io = ImGui::GetIO();
		for (int i = 0; i < IM_ARRAYSIZE(io.MouseDown); i++)
		{
			// If a mouse press event came, always pass it as "mouse held this frame", so we don't miss click-release events that are shorter than 1 frame.
			io.MouseDown[i] = m_inputData.m_mouseButtonPressed[i] || glfwGetMouseButton(m_window, i) != 0;
			m_inputData.m_mouseButtonPressed[i] = false;
		}

		// Update mouse position
		const ImVec2 mouse_pos_backup = io.MousePos;
		io.MousePos = ImVec2(-FLT_MAX, -FLT_MAX);

		const bool focused = glfwGetWindowAttrib(m_window, GLFW_FOCUSED) != 0;
		if (focused)
		{
			if (io.WantSetMousePos)
			{
				glfwSetCursorPos(m_window, (double)mouse_pos_backup.x, (double)mouse_pos_backup.y);
			}
			else
			{
				double mouse_x, mouse_y;
				glfwGetCursorPos(m_window, &mouse_x, &mouse_y);
				io.MousePos = ImVec2((float)mouse_x, (float)mouse_y);
			}
		}

		return false;
	}

	bool Renderer::GlfwUpdateMouseCursor()
	{
		ImGuiIO& io = ImGui::GetIO();
		if ((io.ConfigFlags & ImGuiConfigFlags_NoMouseCursorChange) || glfwGetInputMode(m_window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED)
			return true;

		ImGuiMouseCursor imgui_cursor = ImGui::GetMouseCursor();
		if (imgui_cursor == ImGuiMouseCursor_None || io.MouseDrawCursor)
		{
			// Hide OS mouse cursor if imgui is drawing it or if it wants no cursor
			glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
		}
		else
		{
			// Show OS mouse cursor
			// FIXME-PLATFORM: Unfocused windows seems to fail changing the mouse cursor with GLFW 3.2, but 3.3 works here.
			glfwSetCursor(m_window, m_inputData.m_mouseCursors[imgui_cursor] ? m_inputData.m_mouseCursors[imgui_cursor] : m_inputData.m_mouseCursors[ImGuiMouseCursor_Arrow]);
			glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		}
		return true;
	}

	bool Renderer::GlfwUpdateGamepads()
	{
		ImGuiIO& io = ImGui::GetIO();
		memset(io.NavInputs, 0, sizeof(io.NavInputs));
		if ((io.ConfigFlags & ImGuiConfigFlags_NavEnableGamepad) == 0)
			return true;

		// Update gamepad inputs
#define MAP_BUTTON(NAV_NO, BUTTON_NO)       { if (buttons_count > BUTTON_NO && buttons[BUTTON_NO] == GLFW_PRESS) io.NavInputs[NAV_NO] = 1.0f; }
#define MAP_ANALOG(NAV_NO, AXIS_NO, V0, V1) { float v = (axes_count > AXIS_NO) ? axes[AXIS_NO] : V0; v = (v - V0) / (V1 - V0); if (v > 1.0f) v = 1.0f; if (io.NavInputs[NAV_NO] < v) io.NavInputs[NAV_NO] = v; }
		int axes_count = 0, buttons_count = 0;
		const float* axes = glfwGetJoystickAxes(GLFW_JOYSTICK_1, &axes_count);
		const unsigned char* buttons = glfwGetJoystickButtons(GLFW_JOYSTICK_1, &buttons_count);
		MAP_BUTTON(ImGuiNavInput_Activate, 0);     // Cross / A
		MAP_BUTTON(ImGuiNavInput_Cancel, 1);     // Circle / B
		MAP_BUTTON(ImGuiNavInput_Menu, 2);     // Square / X
		MAP_BUTTON(ImGuiNavInput_Input, 3);     // Triangle / Y
		MAP_BUTTON(ImGuiNavInput_DpadLeft, 13);    // D-Pad Left
		MAP_BUTTON(ImGuiNavInput_DpadRight, 11);    // D-Pad Right
		MAP_BUTTON(ImGuiNavInput_DpadUp, 10);    // D-Pad Up
		MAP_BUTTON(ImGuiNavInput_DpadDown, 12);    // D-Pad Down
		MAP_BUTTON(ImGuiNavInput_FocusPrev, 4);     // L1 / LB
		MAP_BUTTON(ImGuiNavInput_FocusNext, 5);     // R1 / RB
		MAP_BUTTON(ImGuiNavInput_TweakSlow, 4);     // L1 / LB
		MAP_BUTTON(ImGuiNavInput_TweakFast, 5);     // R1 / RB
		MAP_ANALOG(ImGuiNavInput_LStickLeft, 0, -0.3f, -0.9f);
		MAP_ANALOG(ImGuiNavInput_LStickRight, 0, +0.3f, +0.9f);
		MAP_ANALOG(ImGuiNavInput_LStickUp, 1, +0.3f, +0.9f);
		MAP_ANALOG(ImGuiNavInput_LStickDown, 1, -0.3f, -0.9f);
#undef MAP_BUTTON
#undef MAP_ANALOG
		if (axes_count > 0 && buttons_count > 0)
			io.BackendFlags |= ImGuiBackendFlags_HasGamepad;
		else
			io.BackendFlags &= ~ImGuiBackendFlags_HasGamepad;
		
		return true;
	}

	void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
	{
		GlfwInputData* inputData = reinterpret_cast<GlfwInputData*>(glfwGetWindowUserPointer(window));
		if (inputData->m_prevUserCallbackMousebutton != NULL)
			inputData->m_prevUserCallbackMousebutton(window, button, action, mods);

		if (action == GLFW_PRESS && button >= 0 && button < IM_ARRAYSIZE(inputData->m_mouseButtonPressed))
			inputData->m_mouseButtonPressed[button] = true;
	}

	void ScrollCallback(GLFWwindow* window, double xOffset, double yOffset)
	{
		GlfwInputData* inputData = reinterpret_cast<GlfwInputData*>(glfwGetWindowUserPointer(window));
		if (inputData->m_prevUserCallbackScroll != NULL)
			inputData->m_prevUserCallbackScroll(window, xOffset, yOffset);

		ImGuiIO& io = ImGui::GetIO();
		io.MouseWheelH += (float)xOffset;
		io.MouseWheel += (float)yOffset;
	}

	void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
	{
		GlfwInputData* inputData = reinterpret_cast<GlfwInputData*>(glfwGetWindowUserPointer(window));
		if (inputData->m_prevUserCallbackKey != NULL)
			inputData->m_prevUserCallbackKey(window, key, scancode, action, mods);

		ImGuiIO& io = ImGui::GetIO();
		if (action == GLFW_PRESS)
			io.KeysDown[key] = true;
		if (action == GLFW_RELEASE)
			io.KeysDown[key] = false;

		// Modifiers are not reliable across systems
		io.KeyCtrl = io.KeysDown[GLFW_KEY_LEFT_CONTROL] || io.KeysDown[GLFW_KEY_RIGHT_CONTROL];
		io.KeyShift = io.KeysDown[GLFW_KEY_LEFT_SHIFT] || io.KeysDown[GLFW_KEY_RIGHT_SHIFT];
		io.KeyAlt = io.KeysDown[GLFW_KEY_LEFT_ALT] || io.KeysDown[GLFW_KEY_RIGHT_ALT];
		io.KeySuper = io.KeysDown[GLFW_KEY_LEFT_SUPER] || io.KeysDown[GLFW_KEY_RIGHT_SUPER];
	}

	void CharCallback(GLFWwindow* window, unsigned int ch)
	{
		GlfwInputData* inputData = reinterpret_cast<GlfwInputData*>(glfwGetWindowUserPointer(window));
		if (inputData->m_prevUserCallbackChar != NULL)
			inputData->m_prevUserCallbackChar(window, ch);

		ImGui::GetIO().AddInputCharacter(ch);
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
		//TODO: research multiple queue strategies

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