#include "PipelineImGui.h"

namespace MelonRenderer
{
	void PipelineImGui::Init(VkPhysicalDevice& device, DeviceMemoryManager& memoryManager, OutputSurface outputSurface, VkExtent2D windowExtent)
	{
	}
	void PipelineImGui::Tick(float timeDelta)
	{
	}
	void PipelineImGui::RecreateSwapchain(VkExtent2D windowExtent)
	{
	}
	void PipelineImGui::Fini()
	{
	}

	void PipelineImGui::DefineVertices()
	{
		VkVertexInputBindingDescription vertexInputBinding;
		vertexInputBinding.binding = 0;
		vertexInputBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		vertexInputBinding.stride = sizeof(ImDrawVert);
		m_vertexInputBindings.emplace_back(vertexInputBinding);
		

		VkVertexInputAttributeDescription vertexAttributePosition, vertexAttributeUV, vertexAttributeColor;
		vertexAttributePosition.binding = 0;
		vertexAttributePosition.location = 0;
		vertexAttributePosition.format = VK_FORMAT_R32G32_SFLOAT;
		vertexAttributePosition.offset = IM_OFFSETOF(ImDrawVert, pos);
		m_vertexInputAttributes.emplace_back(vertexAttributePosition);

		vertexAttributeUV.binding = 0;
		vertexAttributeUV.location = 1;
		vertexAttributeUV.format = VK_FORMAT_R32G32_SFLOAT;
		vertexAttributeUV.offset = IM_OFFSETOF(ImDrawVert, uv);
		m_vertexInputAttributes.emplace_back(vertexAttributeUV);

		vertexAttributeColor.binding = 0;
		vertexAttributeColor.location = 2;
		vertexAttributeColor.format = VK_FORMAT_R8G8B8A8_UNORM;
		vertexAttributeColor.offset = IM_OFFSETOF(ImDrawVert, col);
		m_vertexInputAttributes.emplace_back(vertexAttributeColor);
	}

	bool PipelineImGui::CreateSwapchain(VkPhysicalDevice& device)
	{
		return false;
	}
	bool PipelineImGui::CleanupSwapchain()
	{
		return false;
	}
	
	bool PipelineImGui::AquireNextImage()
	{
		return false;
	}
	
	bool PipelineImGui::CreateRenderPass()
	{
		VkAttachmentDescription attachments[1];
		attachments[0].format = VK_FORMAT_B8G8R8A8_UNORM; //TODO: check if this is the surface format
		attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
		attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		attachments[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		attachments[0].flags = 0;

		VkAttachmentReference colorReference = {};
		colorReference.attachment = 0;
		colorReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpassDescription = {};
		subpassDescription.flags = 0;
		subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpassDescription.inputAttachmentCount = 0;
		subpassDescription.pInputAttachments = nullptr;
		subpassDescription.colorAttachmentCount = 1;
		subpassDescription.pColorAttachments = &colorReference;
		subpassDescription.pResolveAttachments = nullptr;
		subpassDescription.pDepthStencilAttachment = nullptr;
		subpassDescription.preserveAttachmentCount = 0;
		subpassDescription.pPreserveAttachments = nullptr;

		VkSubpassDependency dependency = {};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		dependency.dependencyFlags = 0;

		VkRenderPassCreateInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.pNext = nullptr;
		renderPassInfo.flags = 0;
		renderPassInfo.attachmentCount = 1;
		renderPassInfo.pAttachments = attachments;
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpassDescription;
		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;

		VkResult result = vkCreateRenderPass(Device::Get().m_device, &renderPassInfo, nullptr, &m_renderPass);
		if (result != VK_SUCCESS)
		{
			Logger::Log("Could not create render pass.");
			return false;
		}

		return true;
	}

	bool PipelineImGui::CreateShaderModules()
	{
		auto vertShaderCode = readFile("shaders/imguiVert.spv");
		auto fragShaderCode = readFile("shaders/imguiFrag.spv");

		VkPipelineShaderStageCreateInfo vertexShader, fragmentShader;

		CreateShaderModule(vertShaderCode, vertexShader.module);
		CreateShaderModule(fragShaderCode, fragmentShader.module);

		vertexShader.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertexShader.pNext = nullptr;
		vertexShader.flags = 0;
		vertexShader.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertexShader.pName = "main";
		vertexShader.pSpecializationInfo = nullptr;

		fragmentShader.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragmentShader.pNext = nullptr;
		fragmentShader.flags = 0;
		fragmentShader.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		fragmentShader.pName = "main";
		fragmentShader.pSpecializationInfo = nullptr;

		m_shaderStagesV.emplace_back(vertexShader);
		m_shaderStagesV.emplace_back(fragmentShader);

		return true;
	}
	bool PipelineImGui::CreateFramebuffers()
	{
		return false;
	}
	bool PipelineImGui::CreateGraphicsPipeline()
	{
		return false;
	}
	bool PipelineImGui::Draw(float timeDelta)
	{
		return false;
	}
	bool PipelineImGui::CreatePipelineLayout()
	{
		return false;
	}
	bool PipelineImGui::CreateDescriptorPool()
	{
		return false;
	}
	bool PipelineImGui::CreateDescriptorSet()
	{
		return false;
	}
}