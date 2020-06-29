#pragma once

#include "Basics.h"
#include "Swapchain.h"

#include <vector>

namespace MelonRenderer
{
	class Renderpass
	{
	public:
		Renderpass(Swapchain* swapchain);

		bool CreateRenderpass();
		bool BeginRenderpass(VkCommandBuffer& commandBuffer);
		bool EndRenderpass(VkCommandBuffer& commandBuffer);

		uint32_t AddAttachment(VkAttachmentDescription& attachment, VkClearValue& clearValue);
		uint32_t AddSubpass(VkSubpassDescription& subpass);
		void AddSubpassDependency(VkSubpassDependency& subpassDependency);

		VkRenderPass* GetVkRenderpass();
		VkAttachmentDescription* GetColorAttachmentPointer();

	protected:
		std::vector<VkAttachmentDescription> m_attachments;
		std::vector<VkSubpassDescription> m_subpasses;
		std::vector<VkSubpassDependency> m_subpassDependencies;
		std::vector<VkClearValue> m_clearValues;

		VkRenderPass m_renderpass;
		Swapchain* m_swapchain;
	};
}
