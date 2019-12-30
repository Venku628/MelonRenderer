#pragma once

#include "loader/VulkanFunctions.h"
#include "Logger.h"
#include "MelonMath.h"


// almost every vulkan function needs access to the logical device
class Device
{
public:
	// singleton
	static Device& Get()
	{
		static Device instance;
		return instance;
	};
	Device(Device const&) = delete;
	void operator=(Device const&) = delete;

	VkDevice m_device;
	VkFormat m_format;
	VkQueue m_multipurposeQueue;
	VkPhysicalDeviceMemoryProperties m_physicalDeviceMemoryProperties;

protected:
	Device() {};
};