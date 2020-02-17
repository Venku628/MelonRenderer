#include "Pipeline.h"

namespace MelonRenderer
{
	bool Pipeline::AquireNextImage()
	{
		/*
		VkResult result = vkAcquireNextImageKHR(Device::Get().m_device, m_swapchain, 2000000000, m_semaphore, m_fence, &m_imageIndex);
		if ((result != VK_SUCCESS) && (result != VK_SUBOPTIMAL_KHR))
		{
			Logger::Log("Could not aquire next image.");
			return false;
		}
		*/

		//TODO: if VK_ERROR_OUT_OF_DATE_KHR, swapchain has to be recreated

		return true;
	}
	bool Pipeline::PresentImage(VkFence* drawFence)
	{
		//TODO: add more parameters
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
		VkResult result;
		if (drawFence != nullptr)
		{
			result = vkWaitForFences(Device::Get().m_device, 1, drawFence, VK_TRUE, UINT64_MAX);
			if (result != VK_SUCCESS)
			{
				Logger::Log("Could not wait for draw fences.");
				return false;
			}
		}

		result = vkQueuePresentKHR(Device::Get().m_multipurposeQueue, &presentInfo);
		if (result != VK_SUCCESS)
		{
			Logger::Log("Could not present the draw queue.");
			return false;
		}

		return true;
	}

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
	bool Pipeline::CleanupSwapchain()
	{
		for (auto& framebuffer : m_framebuffers) {
			vkDestroyFramebuffer(Device::Get().m_device, framebuffer, nullptr);
		}

		for (int i = 0; i < m_commandPools.size(); i++)
		{
			vkFreeCommandBuffers(Device::Get().m_device, m_commandPools[i], static_cast<uint32_t>(1), &m_commandBuffers[i]);
		}

		vkDestroyPipeline(Device::Get().m_device, m_pipeline, nullptr);
		vkDestroyPipelineLayout(Device::Get().m_device, m_pipelineLayout, nullptr);
		vkDestroyRenderPass(Device::Get().m_device, m_renderPass, nullptr);

		for (auto& swapchainImageView : m_swapchainImageViews) {
			vkDestroyImageView(Device::Get().m_device, swapchainImageView, nullptr);
		}

		vkDestroySwapchainKHR(Device::Get().m_device, m_swapchain, nullptr);

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