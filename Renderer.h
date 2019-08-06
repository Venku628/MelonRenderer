#pragma once

#include "loader/VulkanFunctions.h"
#include "Logger.h"

#include <iostream>
#include <vector>

#if defined _WIN32
#include <Windows.h>
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


	private:
		bool LoadVulkanLibrary();
		bool LoadExportedFunctions();
		bool LoadGlobalFunctions();
		bool CheckInstanceExtensions();
		bool CreateInstance();


		std::vector<VkExtensionProperties> m_availableExtensions;
		VULKAN_LIBRARY_TYPE m_vulkanLibrary;
		
	};
}



