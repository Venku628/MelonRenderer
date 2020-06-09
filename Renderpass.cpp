#include "Renderpass.h"

namespace MelonRenderer
{
	Renderpass::Renderpass()
	{
		VkAttachmentDescription colorAttachment = {};
		colorAttachment.format = VK_FORMAT_B8G8R8A8_UNORM; //TODO: parameter
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		//default color attachment is always attachment 0
		m_attachments.emplace_back(colorAttachment);
	}

	bool Renderpass::CreateRenderpass()
	{
		VkRenderPassCreateInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = m_attachments.size();
		renderPassInfo.pAttachments = m_attachments.data();
		renderPassInfo.subpassCount = m_subpasses.size();
		renderPassInfo.pSubpasses = m_subpasses.data();
		renderPassInfo.dependencyCount = m_subpassDependencies.size();
		renderPassInfo.pDependencies = m_subpassDependencies.data();
		VkResult result = vkCreateRenderPass(Device::Get().m_device, &renderPassInfo, nullptr, &m_renderpass);
		if (result != VK_SUCCESS)
		{
			Logger::Log("Could not create renderpass.");
			return false;
		}

		return true;
	}

	bool Renderpass::BeginRenderpass(VkCommandBuffer& commandBuffer)
	{
		VkRenderPassBeginInfo renderPassBegin = {};
		renderPassBegin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassBegin.pNext = nullptr;
		renderPassBegin.renderPass = m_renderpass;
		renderPassBegin.framebuffer = m_swapchain.GetFramebuffer();
		renderPassBegin.renderArea.offset.x = 0;
		renderPassBegin.renderArea.offset.y = 0;
		renderPassBegin.renderArea.extent.width = m_extent.width;
		renderPassBegin.renderArea.extent.height = m_extent.height;
		renderPassBegin.clearValueCount = 0;
		renderPassBegin.pClearValues = nullptr;
		vkCmdBeginRenderPass(commandBuffer, &renderPassBegin, VK_SUBPASS_CONTENTS_INLINE);

		return true;
	}

	bool Renderpass::EndRenderpass(VkCommandBuffer& commandBuffer)
	{
		vkCmdEndRenderPass(commandBuffer);

		return true;
	}

	uint32_t Renderpass::AddAttachment(VkAttachmentDescription& attachment)
	{
		m_attachments.emplace_back(attachment);

		return m_attachments.size() - 1;
	}

	uint32_t Renderpass::AddSubpass(VkSubpassDescription& subpass)
	{
		m_subpasses.emplace_back(subpass);

		return m_subpasses.size() - 1;
	}

	void Renderpass::AddSubpassDependency(VkSubpassDependency& subpassDependency)
	{
		m_subpassDependencies.emplace_back(subpassDependency);
	}
}