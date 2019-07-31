#pragma once

#include <iostream>
#if defined _WIN32
#include <Windows.h>
#elif defined __linux
#include <dlfcn.h>
#endif

#include "loader/VulkanFunctions.h"

int main(int argc, char* argv[]) { 


#if defined _WIN32
	HMODULE vulkanLibrary;
	vulkanLibrary = LoadLibraryA("vulkan-1.dll");
#elif defined __linux
	void* vulkanLibary;
	vulkanLibary = dlopen("libvulkan.so.1", RTLD_NOW);
#endif

	if (vulkanLibrary == nullptr)
	{
		std::cout << "Could not connect with Vulkan runtime library." << std::endl;
		return false;
	}



#if defined _WIN32
#define LoadFunction GetProcAddress
#elif defined __linux
#define LoadFunction dlsym
#endif

	
#define EXPORTED_VULKAN_FUNCTION( name ) name = (PFN_##name)LoadFunction( vulkanLibrary, #name);	\
	if (name == nullptr)																		\
	{std::cout << "Could not load exported function named: " #name << std::endl; return false;}	\

#include "loader/ListOfVulkanFunctions.inl"


	//TODO: load other function types
}
