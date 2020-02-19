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
		std::vector<VkImageView>* GetAttachmentPointer();

		bool AquireNextImage();
		bool PresentImage(VkFence* drawFence = nullptr);

		bool CreateSwapchain(VkPhysicalDevice& device, VkRenderPass* renderPass, OutputSurface outputSurface, VkExtent2D& extent);
		bool CleanupSwapchain(bool preserveSwapchain = true);

	protected:
		std::vector<VkImage> m_outputImages;
		std::vector<VkImageView> m_outputImageViews;
		std::vector<VkFramebuffer> m_framebuffers;
		std::vector<VkCommandPool> m_commandPools;
		std::vector<VkCommandBuffer> m_commandBuffers;
		std::vector<VkSemaphore> m_renderCompleteSemaphores;
		std::vector<VkSemaphore> m_presentCompleteSemaphores;

		bool CreateFramebuffers();
		bool CreateCommandBufferPool(VkCommandPool& commandPool, VkCommandPoolCreateFlags flags);
		bool CreateCommandBuffer(VkCommandPool& commandPool, VkCommandBuffer& commandBuffer);
		bool CreateCommandPoolsAndBuffers();
		bool CreateSemaphores();

		uint32_t m_imageIndex;
		uint32_t m_swapchainSize = 0;
		VkSwapchainKHR m_swapchain;
		VkSwapchainKHR m_oldSwapchain;

		VkExtent2D m_extent;
		VkPresentModeKHR m_presentMode;
		VkFence m_fence;
		const uint32_t m_queueFamilyIndex = 0; //TODO: make variable 
		
		std::vector<VkImageView> m_attachments;
		VkRenderPass* m_renderpass;
		OutputSurface m_outputSurface;
	};
}