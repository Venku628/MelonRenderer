#pragma once

#include "Basics.h"
#include <vector>

namespace MelonRenderer
{
	class Renderpass
	{
	public:
		Renderpass();

		bool CreateRenderpass();
		bool BeginRenderpass(VkCommandBuffer& commandBuffer);
		bool EndRenderpass(VkCommandBuffer& commandBuffer);

		uint32_t AddAttachment(VkAttachmentDescription& attachment);
		uint32_t AddSubpass(VkSubpassDescription& subpass);
		void AddSubpassDependency(VkSubpassDependency& subpassDependency);

	protected:

		std::vector<VkAttachmentDescription> m_attachments;
		std::vector<VkSubpassDescription> m_subpasses;
		std::vector<VkSubpassDependency> m_subpassDependencies;

		VkRenderPass m_renderpass;




	};
}
