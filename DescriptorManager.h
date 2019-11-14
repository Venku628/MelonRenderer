#pragma once

#include <vector>
#include "Basics.h"

namespace MelonRenderer
{
	class DescriptorManager
	{
	public:
		DescriptorManager(VkDevice* logicalDevice);
		bool Init();

	private:
		VkDevice* m_logicalDevice;
		VkDescriptorPool m_descriptorPool;

		uint32_t m_maxSets = 2;

	};
}