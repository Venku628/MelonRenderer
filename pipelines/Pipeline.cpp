#include "Pipeline.h"

namespace MelonRenderer
{
	bool Pipeline::CreateShaderModule(const std::vector<char>& code, VkShaderModule& shaderModule)
	{
		VkShaderModuleCreateInfo shaderModuleCreateInfo = {
				VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
				nullptr,
				0,
				code.size(),
				reinterpret_cast<const uint32_t*>(code.data())
		};
		VkResult result = vkCreateShaderModule(Device::Get().m_device, &shaderModuleCreateInfo, nullptr, &shaderModule);
		if (result != VK_SUCCESS)
		{
			Logger::Log("Could not create shader module.");
			return false;
		}

		return true;
	}

	

	bool Pipeline::CleanupOutput()
	{
		vkDestroyPipeline(Device::Get().m_device, m_pipeline, nullptr);
		vkDestroyPipelineLayout(Device::Get().m_device, m_pipelineLayout, nullptr);

		return true;
	}

	bool Pipeline::CreateCommandBufferPool(VkCommandPool& commandPool, VkCommandPoolCreateFlags flags)
	{
		VkCommandPoolCreateInfo cmdBufferPoolInfo = {};
		cmdBufferPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		cmdBufferPoolInfo.pNext = NULL;
		cmdBufferPoolInfo.queueFamilyIndex = m_queueFamilyIndex;
		cmdBufferPoolInfo.flags = flags;

		VkResult result = vkCreateCommandPool(Device::Get().m_device, &cmdBufferPoolInfo, NULL, &commandPool);
		if (result != VK_SUCCESS)
		{
			Logger::Log("Could not create command pool.");
			return false;
		}

		return true;
	}
	bool Pipeline::CreateCommandBuffer(VkCommandPool& commandPool, VkCommandBuffer& commandBuffer)
	{
		VkCommandBufferAllocateInfo cmdBufferInfo = {};
		cmdBufferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		cmdBufferInfo.pNext = NULL;
		cmdBufferInfo.commandPool = commandPool;
		cmdBufferInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		cmdBufferInfo.commandBufferCount = 1;

		VkResult result = vkAllocateCommandBuffers(Device::Get().m_device, &cmdBufferInfo, &commandBuffer);
		if (result != VK_SUCCESS)
		{
			Logger::Log("Could not create command buffer.");
			return false;
		}

		return true;
	}
}