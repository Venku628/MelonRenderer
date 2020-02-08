#pragma once

#include "loader/VulkanFunctions.h"
#include "Logger.h"
#include "MelonMath.h"

//most vulkan enums are not enum class
#pragma warning(push)
#pragma warning(disable : 26812)

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

protected:
	Device() {};
};