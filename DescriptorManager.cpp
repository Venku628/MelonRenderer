#include "DescriptorManager.h"

namespace MelonRenderer
{
	DescriptorManager::DescriptorManager(VkDevice* logicalDevice)
	{
		m_logicalDevice = logicalDevice;


	}
	bool DescriptorManager::Init()
	{
		
		return true;
		
	}
}