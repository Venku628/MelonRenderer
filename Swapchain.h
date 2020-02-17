#pragma once
#include "Basics.h"
#include <vector>

namespace MelonRenderer
{

	struct OutputSurface
	{
		VkSurfaceCapabilitiesKHR capabilites;
		VkSurfaceKHR surface;
	};

	class Swapchain
	{
	public:
		VkCommandBuffer& GetCommandBuffer();
		VkFramebuffer& GetFramebuffer();

	protected:
		bool AquireNextImage();
		bool PresentImage(VkFence* drawFence = nullptr);

		bool CreateSwapchain(VkPhysicalDevice& device, VkExtent2D& extent);
		bool CleanupSwapchain();

		std::vector<VkImage> m_outputImages;
		std::vector<VkImageView> m_outputImageViews;
		std::vector<VkFramebuffer> m_framebuffers;
		std::vector<VkCommandPool> m_commandPools;
		std::vector<VkCommandBuffer> m_commandBuffers;

		bool CreateFramebuffers();
		bool CreateCommandBufferPool(VkCommandPool& commandPool, VkCommandPoolCreateFlags flags);
		bool CreateCommandBuffer(VkCommandPool& commandPool, VkCommandBuffer& commandBuffer);
		bool CreateCommandPoolsAndBuffers();

		uint32_t m_imageIndex;
		uint32_t m_swapchainSize = 0;
		VkSwapchainKHR m_swapchain;

		VkExtent2D m_extent;
		VkPresentModeKHR m_presentMode;
		VkFence m_fence;
		const uint32_t m_queueFamilyIndex = 0; //TODO: make variable 
		
		std::vector<VkImageView> m_attachments;
		VkRenderPass* m_renderpass;
		OutputSurface m_outputSurface;
	};
}