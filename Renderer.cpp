#include "Renderer.h"

namespace MelonRenderer
{

	void MelonRenderer::Renderer::Init()
	{
		LoadVulkanLibrary();
		LoadExportedFunctions();
		LoadGlobalFunctions();
		CheckInstanceExtensions();

		Logger::Log("Loading complete.");
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

		VkResult result = vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
		if (result != VK_SUCCESS)
		{
			Logger::Log("Could not get number of instance extensions.");
			return false;
		}

		m_availableExtensions.resize(extensionCount);
		result = vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, &m_availableExtensions[0]);
		if (result != VK_SUCCESS)
		{
			Logger::Log("Could not enumerate instance extensions.");
			return false;
		}

		return true;
	}

	bool Renderer::CreateInstance()
	{



		return true;
	}

}