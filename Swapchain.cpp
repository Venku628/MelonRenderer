#include "Swapchain.h"

namespace MelonRenderer
{
	VkCommandBuffer& Swapchain::GetCommandBuffer()
	{
		return m_commandBuffers[m_imageIndex];
	}

	VkFramebuffer& Swapchain::GetFramebuffer()
	{
		return m_framebuffers[m_imageIndex];
	}

	bool Swapchain::AquireNextImage()
	{
		
		VkResult result = vkAcquireNextImageKHR(Device::Get().m_device, m_swapchain, 2000000000, nullptr, nullptr, &m_imageIndex);
		if ((result != VK_SUCCESS) && (result != VK_SUBOPTIMAL_KHR))
		{
			Logger::Log("Could not aquire next image.");
			return false;
		}
		//TODO: if VK_ERROR_OUT_OF_DATE_KHR, swapchain has to be recreated

		return true;
	}

	bool Swapchain::PresentImage(VkFence* drawFence)
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

	bool Swapchain::CreateFramebuffers()
	{
		//imgui uses no depth buffer
		//TODO: get attachments from pipelines maybe?


		VkFramebufferCreateInfo framebufferCreateInfo = {};
		framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferCreateInfo.pNext = nullptr;
		framebufferCreateInfo.flags = 0;
		framebufferCreateInfo.renderPass = *m_renderpass;
		framebufferCreateInfo.attachmentCount = static_cast<uint32_t>(m_attachments.size());
		framebufferCreateInfo.pAttachments = m_attachments.data();
		framebufferCreateInfo.width = m_extent.width;
		framebufferCreateInfo.height = m_extent.height;
		framebufferCreateInfo.layers = 1;

		//the first attachment always is the color output attachment
		for (int i = 0; i < m_swapchainSize; i++)
		{
			m_attachments[0] = m_outputImageViews[i];
			VkResult result = vkCreateFramebuffer(Device::Get().m_device, &framebufferCreateInfo, nullptr, &m_framebuffers[i]);
			if (result != VK_SUCCESS)
			{
				Logger::Log("Could not create framebuffer.");
				return false;
			}
		}

		return true;
	}

	bool Swapchain::CreateCommandBufferPool(VkCommandPool& commandPool, VkCommandPoolCreateFlags flags)
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

	bool Swapchain::CreateCommandBuffer(VkCommandPool& commandPool, VkCommandBuffer& commandBuffer)
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

	bool Swapchain::CreateCommandPoolsAndBuffers()
	{
		for (int i = 0; i < m_swapchainSize; i++)
		{
			CreateCommandBufferPool(m_commandPools[i], VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
			CreateCommandBuffer(m_commandPools[i], m_commandBuffers[i]);
		}

		return true;
	}

	bool Swapchain::CreateSwapchain(VkPhysicalDevice& device, VkExtent2D& extent)
	{
		uint32_t desiredNumberOfImages = 2;

		if (desiredNumberOfImages < m_outputSurface.capabilites.minImageCount)
		{
			desiredNumberOfImages = m_outputSurface.capabilites.minImageCount;
			Logger::Log("Desired number of images too low, using minimum instead.");
		}

		if (desiredNumberOfImages > m_outputSurface.capabilites.maxImageCount)
		{
			desiredNumberOfImages = m_outputSurface.capabilites.maxImageCount;
			Logger::Log("Desired number of images too high, using maximum instead.");
		}

		if (extent.width < m_outputSurface.capabilites.minImageExtent.width)
		{
			extent.width = m_outputSurface.capabilites.minImageExtent.width;
			Logger::Log("Desired width of images too low, using minimum instead.");
		}
		else if (extent.width > m_outputSurface.capabilites.maxImageExtent.width)
		{
			extent.width = m_outputSurface.capabilites.minImageExtent.width;
			Logger::Log("Desired width of images too high, using maximum instead.");
		}

		if (extent.height < m_outputSurface.capabilites.minImageExtent.height)
		{
			extent.height = m_outputSurface.capabilites.minImageExtent.height;
			Logger::Log("Desired height of images too low, using minimum instead.");
		}
		else if (extent.height > m_outputSurface.capabilites.maxImageExtent.height)
		{
			extent.height = m_outputSurface.capabilites.minImageExtent.height;
			Logger::Log("Desired height of images too high, using maximum instead.");
		}

		uint32_t formatCount = 0;
		VkResult result = vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_outputSurface.surface, &formatCount, nullptr);
		if (result != VK_SUCCESS)
		{
			Logger::Log("Could not get number of physical device surface formats.");
			return false;
		}

		std::vector<VkSurfaceFormatKHR> surfaceFormats{ formatCount };
		result = vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_outputSurface.surface, &formatCount, &surfaceFormats[0]);
		if (result != VK_SUCCESS)
		{
			Logger::Log("Could not get physical device surface formats.");
			return false;
		}

		VkFormat imageFormat = surfaceFormats[0].format;
		for (auto& surfaceFormat : surfaceFormats)
		{
			if (surfaceFormat.format == VK_FORMAT_R8G8B8A8_UNORM)
			{
				imageFormat = surfaceFormat.format;
				break;
			}
		}
		Device::Get().m_format = imageFormat;
		VkColorSpaceKHR imageColorSpace = surfaceFormats[0].colorSpace;
		for (auto& surfaceFormat : surfaceFormats)
		{
			if (surfaceFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			{
				imageColorSpace = surfaceFormat.colorSpace;
				break;
			}
		}

		//store values for later use
		m_extent = extent;

		VkSwapchainCreateInfoKHR swapchainCreateInfo = {};
		swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		swapchainCreateInfo.pNext = nullptr;
		swapchainCreateInfo.surface = m_outputSurface.surface;
		swapchainCreateInfo.minImageCount = desiredNumberOfImages;
		swapchainCreateInfo.imageFormat = imageFormat;
		swapchainCreateInfo.imageColorSpace = imageColorSpace;
		swapchainCreateInfo.imageExtent = extent;
		swapchainCreateInfo.imageArrayLayers = 1;
		swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		swapchainCreateInfo.queueFamilyIndexCount = 0;
		swapchainCreateInfo.pQueueFamilyIndices = nullptr;
		swapchainCreateInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
		swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		swapchainCreateInfo.presentMode = m_presentMode;
		swapchainCreateInfo.clipped = VK_TRUE;
		swapchainCreateInfo.oldSwapchain = VK_NULL_HANDLE; //TODO: insert old swapchain if available and destroy after


		VkBool32 deviceSupported;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, m_queueFamilyIndex, m_outputSurface.surface, &deviceSupported);
		if (!deviceSupported)
		{
			Logger::Log("Presentation surface not supported by physical device.");
			return false;
		}

		result = vkCreateSwapchainKHR(Device::Get().m_device, &swapchainCreateInfo, nullptr, &m_swapchain);
		if (result != VK_SUCCESS)
		{
			Logger::Log("Could not create swapchain.");
			return false;
		}

		// driver may create more images than requested
		result = vkGetSwapchainImagesKHR(Device::Get().m_device, m_swapchain, &m_swapchainSize, nullptr);
		if (result != VK_SUCCESS)
		{
			Logger::Log("Could not get number of swapchain images.");
			return false;
		}

		m_outputImages.resize(m_swapchainSize);
		m_outputImageViews.resize(m_swapchainSize);
		m_framebuffers.resize(m_swapchainSize);
		m_commandPools.resize(m_swapchainSize);
		m_commandBuffers.resize(m_swapchainSize);

		result = vkGetSwapchainImagesKHR(Device::Get().m_device, m_swapchain, &m_swapchainSize, &m_outputImages[0]);
		if (result != VK_SUCCESS)
		{
			Logger::Log("Could not enumerate swapchain images.");
			return false;
		}

		VkImageViewCreateInfo colorImageView = {};
		colorImageView.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		colorImageView.pNext = NULL;
		colorImageView.flags = 0;
		//colorImageView.image = m_outputImages[i]      //in for loop below 
		colorImageView.viewType = VK_IMAGE_VIEW_TYPE_2D;
		colorImageView.format = imageFormat;
		colorImageView.components.r = VK_COMPONENT_SWIZZLE_R;
		colorImageView.components.g = VK_COMPONENT_SWIZZLE_G;
		colorImageView.components.b = VK_COMPONENT_SWIZZLE_B;
		colorImageView.components.a = VK_COMPONENT_SWIZZLE_A;
		colorImageView.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		colorImageView.subresourceRange.baseMipLevel = 0;
		colorImageView.subresourceRange.levelCount = 1;
		colorImageView.subresourceRange.baseArrayLayer = 0;
		colorImageView.subresourceRange.layerCount = 1;

		for (unsigned int i = 0; i < m_swapchainSize; i++)
		{
			colorImageView.image = m_outputImages[i];

			result = vkCreateImageView(Device::Get().m_device, &colorImageView, nullptr, &m_outputImageViews[i]);
			if (result != VK_SUCCESS)
			{
				Logger::Log("Could not create image view.");
				return false;
			}
		}

		return true;
	}

	bool Swapchain::CleanupSwapchain()
	{
		for (auto& swapchainImageView : m_outputImageViews) {
			vkDestroyImageView(Device::Get().m_device, swapchainImageView, nullptr);
		}
		vkDestroySwapchainKHR(Device::Get().m_device, m_swapchain, nullptr);

		return true;
	}
}