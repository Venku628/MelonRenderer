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
		uint32_t desiredNumberOfImages = 2;

		VkExtent2D desiredSizeOfImages = {};
		desiredSizeOfImages.width = m_extent.width;
		desiredSizeOfImages.height = m_extent.height;

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

		if (desiredSizeOfImages.width < m_outputSurface.capabilites.minImageExtent.width)
		{
			desiredSizeOfImages.width = m_outputSurface.capabilites.minImageExtent.width;
			Logger::Log("Desired width of images too low, using minimum instead.");
		}
		else if (desiredSizeOfImages.width > m_outputSurface.capabilites.maxImageExtent.width)
		{
			desiredSizeOfImages.width = m_outputSurface.capabilites.minImageExtent.width;
			Logger::Log("Desired width of images too high, using maximum instead.");
		}

		if (desiredSizeOfImages.height < m_outputSurface.capabilites.minImageExtent.height)
		{
			desiredSizeOfImages.height = m_outputSurface.capabilites.minImageExtent.height;
			Logger::Log("Desired height of images too low, using minimum instead.");
		}
		else if (desiredSizeOfImages.height > m_outputSurface.capabilites.maxImageExtent.height)
		{
			desiredSizeOfImages.height = m_outputSurface.capabilites.minImageExtent.height;
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
		m_extent.height = desiredSizeOfImages.height;
		m_extent.width = desiredSizeOfImages.width;

		VkSwapchainCreateInfoKHR swapchainCreateInfo = {};
		swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		swapchainCreateInfo.pNext = nullptr;
		swapchainCreateInfo.surface = m_outputSurface.surface;
		swapchainCreateInfo.minImageCount = desiredNumberOfImages;
		swapchainCreateInfo.imageFormat = imageFormat;
		swapchainCreateInfo.imageColorSpace = imageColorSpace;
		swapchainCreateInfo.imageExtent = desiredSizeOfImages;
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
		uint32_t actualImageCount = 0;
		result = vkGetSwapchainImagesKHR(Device::Get().m_device, m_swapchain, &actualImageCount, nullptr);
		if (result != VK_SUCCESS)
		{
			Logger::Log("Could not get number of swapchain images.");
			return false;
		}

		m_swapchainImages.resize(actualImageCount);
		result = vkGetSwapchainImagesKHR(Device::Get().m_device, m_swapchain, &actualImageCount, &m_swapchainImages[0]);
		if (result != VK_SUCCESS)
		{
			Logger::Log("Could not enumerate swapchain images.");
			return false;
		}

		m_swapchainImageViews.resize(actualImageCount);

		VkImageViewCreateInfo colorImageView = {};
		colorImageView.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		colorImageView.pNext = NULL;
		colorImageView.flags = 0;
		//colorImageView.image = m_swapchainImages[i]      in for below
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

		for (unsigned int i = 0; i < actualImageCount; i++)
		{
			colorImageView.image = m_swapchainImages[i];

			result = vkCreateImageView(Device::Get().m_device, &colorImageView, nullptr, &m_swapchainImageViews[i]);
			if (result != VK_SUCCESS)
			{
				Logger::Log("Could not create image view.");
				return false;
			}
		}

		return true;
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
		//imgui uses no depth buffer
		VkImageView attachments[1];

		VkFramebufferCreateInfo framebufferCreateInfo = {};
		framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferCreateInfo.pNext = nullptr;
		framebufferCreateInfo.flags = 0;
		framebufferCreateInfo.renderPass = m_renderPass;
		framebufferCreateInfo.attachmentCount = 1;
		framebufferCreateInfo.pAttachments = attachments;
		framebufferCreateInfo.width = m_extent.width;
		framebufferCreateInfo.height = m_extent.height;
		framebufferCreateInfo.layers = 1;

		m_framebuffers.resize(m_swapchainImages.size());
		for (int i = 0; i < m_swapchainImages.size(); i++)
		{
			attachments[0] = m_swapchainImageViews[i];
			VkResult result = vkCreateFramebuffer(Device::Get().m_device, &framebufferCreateInfo, nullptr, &m_framebuffers[i]);
			if (result != VK_SUCCESS)
			{
				Logger::Log("Could not create framebuffer.");
				return false;
			}
		}

		return true;
	}
	bool PipelineImGui::CreateGraphicsPipeline()
	{
		VkDynamicState dynamicStateEnables[2] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
		VkPipelineDynamicStateCreateInfo dynamicState = {};
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.pNext = nullptr;
		dynamicState.dynamicStateCount = 2;
		dynamicState.pDynamicStates = dynamicStateEnables;

		VkPipelineVertexInputStateCreateInfo pipelineVertexInputInfo = {};
		pipelineVertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		pipelineVertexInputInfo.pNext = nullptr;
		pipelineVertexInputInfo.flags = 0;
		pipelineVertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(m_vertexInputBindings.size());
		pipelineVertexInputInfo.pVertexBindingDescriptions = m_vertexInputBindings.data();
		pipelineVertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(m_vertexInputAttributes.size());
		pipelineVertexInputInfo.pVertexAttributeDescriptions = m_vertexInputAttributes.data();

		VkPipelineInputAssemblyStateCreateInfo pipelineInputAssemblyInfo = {};
		pipelineInputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		pipelineInputAssemblyInfo.pNext = nullptr;
		pipelineInputAssemblyInfo.flags = 0;
		pipelineInputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

		VkPipelineRasterizationStateCreateInfo pipelineRasterizationInfo;
		pipelineRasterizationInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		pipelineRasterizationInfo.pNext = nullptr;
		pipelineRasterizationInfo.flags = 0;
		pipelineRasterizationInfo.polygonMode = VK_POLYGON_MODE_FILL;
		pipelineRasterizationInfo.cullMode = VK_CULL_MODE_NONE;
		pipelineRasterizationInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		pipelineRasterizationInfo.lineWidth = 1.0f;

		VkPipelineColorBlendAttachmentState colorBlendAttachment[1];
		colorBlendAttachment[0].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment[0].blendEnable = VK_TRUE;
		colorBlendAttachment[0].alphaBlendOp = VK_BLEND_OP_ADD;
		colorBlendAttachment[0].colorBlendOp = VK_BLEND_OP_ADD;
		colorBlendAttachment[0].srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		colorBlendAttachment[0].dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		colorBlendAttachment[0].srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		colorBlendAttachment[0].dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;

		VkPipelineColorBlendStateCreateInfo pipelineColorBlendInfo;
		pipelineColorBlendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		pipelineColorBlendInfo.pNext = nullptr;
		pipelineColorBlendInfo.flags = 0;
		pipelineColorBlendInfo.attachmentCount = 1;
		pipelineColorBlendInfo.pAttachments = colorBlendAttachment;

		VkPipelineViewportStateCreateInfo pipelineViewportInfo = {};
		pipelineViewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		pipelineViewportInfo.pNext = nullptr;
		pipelineViewportInfo.flags = 0;
		pipelineViewportInfo.viewportCount = 1;
		pipelineViewportInfo.scissorCount = 1;

		VkPipelineDepthStencilStateCreateInfo pipelineDepthStencilInfo;
		pipelineDepthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		pipelineDepthStencilInfo.pNext = nullptr;
		pipelineDepthStencilInfo.flags = 0;
		pipelineDepthStencilInfo.depthTestEnable = VK_FALSE;
		pipelineDepthStencilInfo.depthWriteEnable = VK_FALSE;

		//setup for no multisampling for now, imgui does not profit a lot from MSAA according to it´s documentation
		VkPipelineMultisampleStateCreateInfo pipelineMultisampleInfo;
		pipelineMultisampleInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		pipelineMultisampleInfo.pNext = nullptr;
		pipelineMultisampleInfo.flags = 0;
		pipelineMultisampleInfo.pSampleMask = NULL;
		pipelineMultisampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT; 
		pipelineMultisampleInfo.sampleShadingEnable = VK_FALSE;
		pipelineMultisampleInfo.alphaToCoverageEnable = VK_FALSE;
		pipelineMultisampleInfo.alphaToOneEnable = VK_FALSE;
		pipelineMultisampleInfo.minSampleShading = 0.0;

		VkGraphicsPipelineCreateInfo pipeline;
		pipeline.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipeline.pNext = nullptr;
		pipeline.flags = 0;
		pipeline.layout = m_pipelineLayout;
		pipeline.basePipelineHandle = VK_NULL_HANDLE;
		pipeline.basePipelineIndex = 0;
		pipeline.pVertexInputState = &pipelineVertexInputInfo;
		pipeline.pInputAssemblyState = &pipelineInputAssemblyInfo;
		pipeline.pRasterizationState = &pipelineRasterizationInfo;
		pipeline.pColorBlendState = &pipelineColorBlendInfo;
		pipeline.pTessellationState = nullptr;
		pipeline.pMultisampleState = &pipelineMultisampleInfo;
		pipeline.pDynamicState = &dynamicState;
		pipeline.pViewportState = &pipelineViewportInfo;
		pipeline.pDepthStencilState = &pipelineDepthStencilInfo;
		pipeline.pStages = m_shaderStagesV.data();
		pipeline.stageCount = static_cast<uint32_t>(m_shaderStagesV.size());
		pipeline.renderPass = m_renderPass;
		pipeline.subpass = 0;

		VkResult result = vkCreateGraphicsPipelines(Device::Get().m_device, VK_NULL_HANDLE, 1, &pipeline, nullptr, &m_pipeline);
		if (result != VK_SUCCESS)
		{
			Logger::Log("Could not create graphics pipeline.");
			return false;
		}

		return true;
	}
	bool PipelineImGui::Draw(float timeDelta)
	{
		return false;
	}
	bool PipelineImGui::CreatePipelineLayout()
	{
		if (!m_fontSampler)
		{
			CreateFontSampler();
		}

		VkDescriptorSetLayoutBinding binding[1] = {};
		binding[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		binding[0].descriptorCount = 1;
		binding[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		binding[0].pImmutableSamplers = &m_fontSampler;

		VkDescriptorSetLayoutCreateInfo descriptorLayoutInfo = {
			VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
			nullptr,
			0,
			1,
			binding
		};
		m_descriptorSetLayouts.resize(1);
		VkResult result = vkCreateDescriptorSetLayout(Device::Get().m_device, &descriptorLayoutInfo, nullptr, m_descriptorSetLayouts.data());
		if (result != VK_SUCCESS)
		{
			Logger::Log("Could not create uniform buffer descriptor set layout.");
			return false;
		}

		VkPushConstantRange pushConstants[1] = {};
		pushConstants[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		pushConstants[0].offset = sizeof(float) * 0;
		pushConstants[0].size = sizeof(float) * 4;

		VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {
			VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
			nullptr,
			0,
			m_descriptorSetLayouts.size(),
			m_descriptorSetLayouts.data(),
			1,
			pushConstants
		};
		result = vkCreatePipelineLayout(Device::Get().m_device, &pipelineLayoutCreateInfo, nullptr, &m_pipelineLayout);
		if (result != VK_SUCCESS)
		{
			Logger::Log("Could not create pipeline layout.");
			return false;
		}

		return true;
	}
	bool PipelineImGui::CreateDescriptorPool()
	{
		VkDescriptorPoolSize descriptorPoolSizes[1];
		descriptorPoolSizes[0].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorPoolSizes[0].descriptorCount = 1;

		VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = {
			VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
			nullptr,
			0,
			1,
			1,
			descriptorPoolSizes
		};
		VkResult result = vkCreateDescriptorPool(Device::Get().m_device, &descriptorPoolCreateInfo, nullptr, &m_descriptorPool);
		if (result != VK_SUCCESS)
		{
			Logger::Log("Could not create descriptor pool.");
			return false;
		}

		return true;
	}
	bool PipelineImGui::CreateDescriptorSet()
	{
		VkDescriptorSetAllocateInfo allocInfo;
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.pNext = nullptr;
		allocInfo.descriptorPool = m_descriptorPool;
		allocInfo.descriptorSetCount = m_descriptorSetLayouts.size(); //m_descriptorSetLayouts.size()
		allocInfo.pSetLayouts = m_descriptorSetLayouts.data();
		m_descriptorSets.resize(m_descriptorSetLayouts.size());
		VkResult result = vkAllocateDescriptorSets(Device::Get().m_device, &allocInfo, m_descriptorSets.data());
		if (result != VK_SUCCESS)
		{
			Logger::Log("Could not allocate descriptor set.");
			return false;
		}

		VkDescriptorImageInfo descriptorImageInfo[1] = {};
		descriptorImageInfo[0].sampler = m_fontSampler;
		descriptorImageInfo[0].imageView = m_fontImageView;
		descriptorImageInfo[0].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		VkWriteDescriptorSet writes[1];
		writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writes[0].pNext = nullptr;
		writes[0].dstSet = m_descriptorSets[0];
		writes[0].descriptorCount = 1;
		writes[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		writes[0].pImageInfo = descriptorImageInfo;
		writes[0].dstArrayElement = 0;
		writes[0].dstBinding = 0;

		vkUpdateDescriptorSets(Device::Get().m_device, 1, writes, 0, nullptr);

		return true;
	}
	bool PipelineImGui::CreateFontSampler()
	{
		VkSamplerCreateInfo samplerCreateInfo = {};
		samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerCreateInfo.magFilter = VK_FILTER_LINEAR;
		samplerCreateInfo.minFilter = VK_FILTER_LINEAR;
		samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerCreateInfo.minLod = -1000;
		samplerCreateInfo.maxLod = 1000;
		samplerCreateInfo.maxAnisotropy = 1.0f;
		VkResult result = vkCreateSampler(Device::Get().m_device, &samplerCreateInfo, nullptr, &m_fontSampler);
		if (result != VK_SUCCESS)
		{
			Logger::Log("Could not create font sampler.");
			return false;
		}

		return true;
	}
}