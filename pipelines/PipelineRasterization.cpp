#include "PipelineRasterization.h"

namespace MelonRenderer
{
	void PipelineRasterization::Init(VkPhysicalDevice& physicalDevice, DeviceMemoryManager& memoryManager, VkRenderPass& renderPass, VkExtent2D windowExtent)
	{
		m_memoryManager = &memoryManager;
		m_renderpass = &renderPass;
		m_extent = windowExtent;
		
		DefineVertices();

		CreateDepthBuffer();
		CreateDynamicTransformBuffer();

		CreatePipelineLayout();
		CreateDescriptorPool();
		CreateDescriptorSets();

		CreateShaderModules();

		CreateGraphicsPipeline();
	}

	void PipelineRasterization::Tick(VkCommandBuffer& commandBuffer)
	{
		m_memoryManager->UpdateDynamicUBO(m_dynamicTransformBuffer);
		Draw(commandBuffer);
	}

	void PipelineRasterization::FillRenderpassInfo(Renderpass* renderpass)
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
		*renderpass->GetColorAttachmentPointer() = colorAttachment;

		// Depth attachment
		VkAttachmentDescription depthAttachment;
		depthAttachment.format = m_depthBufferFormat;
		depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkAttachmentReference colorAttachmentReference = {};
		colorAttachmentReference.attachment = 0;
		colorAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		m_attachmentReferences.emplace_back(colorAttachmentReference);

		VkClearValue depthClearValue = {};
		depthClearValue.depthStencil = {1.f, 0};
		m_depthAttachmentReference.attachment = renderpass->AddAttachment(depthAttachment, depthClearValue);
		m_depthAttachmentReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass = {};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = m_attachmentReferences.size();
		subpass.pColorAttachments = m_attachmentReferences.data();
		subpass.pDepthStencilAttachment = &m_depthAttachmentReference;

		uint32_t subpassNumber = renderpass->AddSubpass(subpass);

		VkSubpassDependency dependency;
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = subpassNumber;
		dependency.srcStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		dependency.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		dependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

		renderpass->AddSubpassDependency(dependency);
	}

	void PipelineRasterization::RecreateOutput(VkExtent2D& windowExtent)
	{
		vkDeviceWaitIdle(Device::Get().m_device);
		CleanupDepthBuffer();

		m_extent = windowExtent;
		CreateDepthBuffer();
	}

	void PipelineRasterization::SetCamera(Camera* camera)
	{
		m_camera = camera;
	}

	void PipelineRasterization::SetScene(Scene* scene)
	{
		m_scene = scene;
	}

	void PipelineRasterization::Fini()
	{
		vkDestroyPipelineLayout(Device::Get().m_device, m_pipelineLayout, nullptr);
		vkDestroyPipeline(Device::Get().m_device, m_pipeline, nullptr);

		free(m_dynamicTransformBuffer.m_uploadBuffer);
	}

	void PipelineRasterization::DefineVertices()
	{
		VkVertexInputBindingDescription vertexInputBinding;
		vertexInputBinding.binding = 0;
		vertexInputBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		vertexInputBinding.stride = sizeof(Vertex);
		m_vertexInputBindings.emplace_back(vertexInputBinding);

		VkVertexInputAttributeDescription vertexAttributePosition, vertexAttributeNormal, vertexAttributeUV, vertexAttributeMaterial;
		vertexAttributePosition.binding = 0;
		vertexAttributePosition.location = 0;
		vertexAttributePosition.format = VK_FORMAT_R32G32B32_SFLOAT;
		vertexAttributePosition.offset = 0;
		m_vertexInputAttributes.emplace_back(vertexAttributePosition);

		vertexAttributeNormal.binding = 0;
		vertexAttributeNormal.location = 1;
		vertexAttributeNormal.format = VK_FORMAT_R32G32B32_SFLOAT;
		vertexAttributeNormal.offset = sizeof(float) * 3;
		m_vertexInputAttributes.emplace_back(vertexAttributeNormal);

		vertexAttributeUV.binding = 0;
		vertexAttributeUV.location = 2;
		vertexAttributeUV.format = VK_FORMAT_R32G32_SFLOAT;
		vertexAttributeUV.offset = sizeof(float) * 6;
		m_vertexInputAttributes.emplace_back(vertexAttributeUV);

		vertexAttributeMaterial.binding = 0;
		vertexAttributeMaterial.location = 3;
		vertexAttributeMaterial.format = VK_FORMAT_R32_UINT;
		vertexAttributeMaterial.offset = sizeof(float) * 8;
		m_vertexInputAttributes.emplace_back(vertexAttributeMaterial);
	}

	bool PipelineRasterization::CreateDepthBuffer()
	{
		VkExtent3D depthExtent;
		depthExtent.width = m_extent.width;
		depthExtent.height = m_extent.height;
		depthExtent.depth = 1;

		VkImageCreateInfo imageCreateInfo = {
		VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		nullptr,
		0,
		VK_IMAGE_TYPE_2D,
		m_depthBufferFormat,
		depthExtent,
		1,
		1,
		VK_SAMPLE_COUNT_1_BIT,
		VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
		VK_SHARING_MODE_EXCLUSIVE,
		0, //family queue index count
		nullptr,
		VK_IMAGE_LAYOUT_UNDEFINED
		};

		VkResult result = vkCreateImage(Device::Get().m_device, &imageCreateInfo, nullptr, &m_depthBuffer);
		if (result != VK_SUCCESS)
		{
			Logger::Log("Could not create image for depth buffer.");
			return false;
		}

		VkMemoryRequirements memoryReq;
		vkGetImageMemoryRequirements(Device::Get().m_device, m_depthBuffer, &memoryReq);

		VkMemoryAllocateInfo memoryAllocInfo =
		{
			VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
			nullptr,
			memoryReq.size,
			0
		};
		if (!m_memoryManager->FindMemoryTypeFromProperties(memoryReq.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &memoryAllocInfo.memoryTypeIndex))
		{
			Logger::Log("Could not find matching memory type from propterties.");
			return false;
		}

		vkAllocateMemory(Device::Get().m_device, &memoryAllocInfo, nullptr, &m_depthBufferMemory);
		vkBindImageMemory(Device::Get().m_device, m_depthBuffer, m_depthBufferMemory, 0);

		VkImageViewCreateInfo viewInfo = {};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.pNext = nullptr;
		viewInfo.flags = 0;
		viewInfo.image = m_depthBuffer;
		viewInfo.format = m_depthBufferFormat;
		viewInfo.components.r = VK_COMPONENT_SWIZZLE_R;
		viewInfo.components.g = VK_COMPONENT_SWIZZLE_G;
		viewInfo.components.b = VK_COMPONENT_SWIZZLE_B;
		viewInfo.components.a = VK_COMPONENT_SWIZZLE_A;
		viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;

		result = vkCreateImageView(Device::Get().m_device, &viewInfo, nullptr, &m_depthBufferView);
		if (result != VK_SUCCESS)
		{
			Logger::Log("Could not create image view for depth buffer.");
			return false;
		}

		return true;
	}

	bool PipelineRasterization::CleanupDepthBuffer()
	{
		vkDestroyImageView(Device::Get().m_device, m_depthBufferView, nullptr);
		vkFreeMemory(Device::Get().m_device, m_depthBufferMemory, nullptr);
		vkDestroyImage(Device::Get().m_device, m_depthBuffer, nullptr);

		return true;
	}

	bool PipelineRasterization::CreateShaderModules()
	{
		auto vertShaderCode = readFile("shaders/vert.spv");
		auto fragShaderCode = readFile("shaders/frag.spv");

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

	bool PipelineRasterization::CreateGraphicsPipeline()
	{
		VkDynamicState dynamicStateEnables[VK_DYNAMIC_STATE_RANGE_SIZE];
		VkPipelineDynamicStateCreateInfo dynamicState = {};
		memset(dynamicStateEnables, 0, sizeof(dynamicStateEnables));
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.pNext = nullptr;
		dynamicState.pDynamicStates = dynamicStateEnables;
		dynamicState.dynamicStateCount = 0;

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
		pipelineInputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

		VkPipelineRasterizationStateCreateInfo pipelineRasterizationInfo = {};
		pipelineRasterizationInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		pipelineRasterizationInfo.pNext = nullptr;
		pipelineRasterizationInfo.flags = 0;
		pipelineRasterizationInfo.polygonMode = VK_POLYGON_MODE_FILL;
		pipelineRasterizationInfo.cullMode = VK_CULL_MODE_BACK_BIT;
		pipelineRasterizationInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
		pipelineRasterizationInfo.depthClampEnable = VK_TRUE;
		pipelineRasterizationInfo.rasterizerDiscardEnable = VK_FALSE;
		pipelineRasterizationInfo.depthBiasEnable = VK_FALSE;
		pipelineRasterizationInfo.depthBiasConstantFactor = 0;
		pipelineRasterizationInfo.depthBiasClamp = 0;
		pipelineRasterizationInfo.depthBiasSlopeFactor = 0;
		pipelineRasterizationInfo.lineWidth = 1.0f;

		VkPipelineColorBlendAttachmentState colorBlendAttachment[1];
		colorBlendAttachment[0].colorWriteMask = 0xf;
		colorBlendAttachment[0].blendEnable = VK_FALSE;
		colorBlendAttachment[0].alphaBlendOp = VK_BLEND_OP_ADD;
		colorBlendAttachment[0].colorBlendOp = VK_BLEND_OP_ADD;
		colorBlendAttachment[0].srcColorBlendFactor = VK_BLEND_FACTOR_ZERO;
		colorBlendAttachment[0].dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
		colorBlendAttachment[0].srcAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		colorBlendAttachment[0].dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;

		VkPipelineColorBlendStateCreateInfo pipelineColorBlendInfo;
		pipelineColorBlendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		pipelineColorBlendInfo.pNext = nullptr;
		pipelineColorBlendInfo.flags = 0;
		pipelineColorBlendInfo.attachmentCount = 1;
		pipelineColorBlendInfo.pAttachments = colorBlendAttachment;
		pipelineColorBlendInfo.logicOpEnable = VK_FALSE;
		pipelineColorBlendInfo.logicOp = VK_LOGIC_OP_NO_OP;
		pipelineColorBlendInfo.blendConstants[0] = 1.0f;
		pipelineColorBlendInfo.blendConstants[1] = 1.0f;
		pipelineColorBlendInfo.blendConstants[2] = 1.0f;
		pipelineColorBlendInfo.blendConstants[3] = 1.0f;

		VkPipelineViewportStateCreateInfo pipelineViewportInfo = {};
		pipelineViewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		pipelineViewportInfo.pNext = nullptr;
		pipelineViewportInfo.flags = 0;
		pipelineViewportInfo.viewportCount = 1;
		dynamicStateEnables[dynamicState.dynamicStateCount++] = VK_DYNAMIC_STATE_VIEWPORT;
		pipelineViewportInfo.scissorCount = 1;
		dynamicStateEnables[dynamicState.dynamicStateCount++] = VK_DYNAMIC_STATE_SCISSOR;
		pipelineViewportInfo.pScissors = nullptr;
		pipelineViewportInfo.pViewports = nullptr;

		VkPipelineDepthStencilStateCreateInfo pipelineDepthStencilInfo;
		pipelineDepthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		pipelineDepthStencilInfo.pNext = nullptr;
		pipelineDepthStencilInfo.flags = 0;
		pipelineDepthStencilInfo.depthTestEnable = VK_TRUE;
		pipelineDepthStencilInfo.depthWriteEnable = VK_TRUE;
		pipelineDepthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
		pipelineDepthStencilInfo.depthBoundsTestEnable = VK_FALSE;
		pipelineDepthStencilInfo.minDepthBounds = 0;
		pipelineDepthStencilInfo.maxDepthBounds = 0;
		pipelineDepthStencilInfo.stencilTestEnable = VK_FALSE;
		pipelineDepthStencilInfo.back.failOp = VK_STENCIL_OP_KEEP;
		pipelineDepthStencilInfo.back.passOp = VK_STENCIL_OP_KEEP;
		pipelineDepthStencilInfo.back.compareOp = VK_COMPARE_OP_ALWAYS;
		pipelineDepthStencilInfo.back.compareMask = 0;
		pipelineDepthStencilInfo.back.reference = 0;
		pipelineDepthStencilInfo.back.depthFailOp = VK_STENCIL_OP_KEEP;
		pipelineDepthStencilInfo.back.writeMask = 0;
		pipelineDepthStencilInfo.front = pipelineDepthStencilInfo.back;

		//setup for no multisampling for now
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
		pipeline.layout = m_pipelineLayout;
		pipeline.basePipelineHandle = VK_NULL_HANDLE;
		pipeline.basePipelineIndex = 0;
		pipeline.flags = 0;
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
		pipeline.renderPass = *m_renderpass;
		pipeline.subpass = 0;

		VkResult result = vkCreateGraphicsPipelines(Device::Get().m_device, VK_NULL_HANDLE, 1, &pipeline, nullptr, &m_pipeline);
		if (result != VK_SUCCESS)
		{
			Logger::Log("Could not create graphics pipeline.");
			return false;
		}

		return true;
	}

	bool PipelineRasterization::CreateDynamicTransformBuffer()
	{
		//TODO: uncouple size from number of instances, take fixed value instead and increase if needed? decide with memory allocator
		m_dynamicTransformBuffer.m_numberOfElements = m_scene->m_drawableInstances.size();
		m_dynamicTransformBuffer.m_alignment = sizeof(DrawableInstance);

		if (!m_memoryManager->CreateDynamicUBO(m_dynamicTransformBuffer))
		{
			Logger::Log("Could not create dynamic transform buffer.");
			return false;
		}

		return true;
	}

	bool PipelineRasterization::UpdateDynamicTransformBuffer()
	{
		for (int i = 0; i < m_scene->m_drawableInstances.size(); i++)
		{
			DrawableInstance* mat = (DrawableInstance*)(((uint64_t)m_dynamicTransformBuffer.m_uploadBuffer + 
				(i * m_dynamicTransformBuffer.m_alignment)));
			*mat = m_scene->m_drawableInstances.operator[](i);
		}

		if (!m_memoryManager->UpdateDynamicUBO(m_dynamicTransformBuffer))
		{
			Logger::Log("Could not update dynamic uniform buffer.");
			return false;
		}

		return true;
	}

	bool PipelineRasterization::Draw(VkCommandBuffer& commandBuffer)
	{
		UpdateDynamicTransformBuffer();

		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);

		uint32_t dynamicOffset = 0;
		
		m_viewport.height = (float)m_extent.height;
		m_viewport.width = (float)m_extent.width;
		m_viewport.minDepth = (float)0.0f;
		m_viewport.maxDepth = (float)1.0f;
		m_viewport.x = 0;
		m_viewport.y = 0;
		vkCmdSetViewport(commandBuffer, 0, 1, &m_viewport);

		m_scissorRect2D.extent.width = m_extent.width;
		m_scissorRect2D.extent.height = m_extent.height;
		m_scissorRect2D.offset.x = 0;
		m_scissorRect2D.offset.y = 0;
		vkCmdSetScissor(commandBuffer, 0, 1, &m_scissorRect2D);

		//TODO: reimplement as soon as refactor is finished
		//m_scene->Tick(commandBuffer);
		VkDeviceSize offsets[1] = { 0 };

		for (int i = 0; i < m_scene->m_drawableInstances.size(); i++)
		{
			Drawable* drawable = &m_scene->m_drawables[m_scene->m_drawableInstances[i].m_drawableIndex];

			vkCmdBindVertexBuffers(commandBuffer, 0, 1, &drawable->m_vertexBuffer, 
				offsets);
			vkCmdBindIndexBuffer(commandBuffer, drawable->m_indexBuffer,
				0, VK_INDEX_TYPE_UINT32);

			uint32_t dynamicOffset = i * m_dynamicTransformBuffer.m_alignment;

			vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineLayout, 0, 1, m_descriptorSets.data(), 
				1, &dynamicOffset);

			vkCmdDrawIndexed(commandBuffer, drawable->m_indexCount, 1, 0, 0, 0);
		}

		return true;
	}

	bool PipelineRasterization::CreatePipelineLayout()
	{
		std::vector<VkDescriptorSetLayoutBinding> layoutBindings;
		unsigned int layoutBindingIndex = 0;

		VkDescriptorSetLayoutBinding layoutBindingViewProjection = {
			layoutBindingIndex++,
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			1,
			VK_SHADER_STAGE_VERTEX_BIT,
			nullptr
		};
		layoutBindings.emplace_back(layoutBindingViewProjection);

		VkDescriptorSetLayoutBinding layoutBindingDynamicTransformUBO = {
			layoutBindingIndex++,
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
			1,
			VK_SHADER_STAGE_VERTEX_BIT,
			nullptr
		};
		layoutBindings.emplace_back(layoutBindingDynamicTransformUBO);

		VkDescriptorSetLayoutBinding materialsLayoutBinding = {
			layoutBindingIndex++,
			VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
			m_scene->m_drawables.size(), //TODO: update this when number of objects changes
			VK_SHADER_STAGE_VERTEX_BIT,
			nullptr
		};
		layoutBindings.emplace_back(materialsLayoutBinding);

		VkDescriptorSetLayoutBinding samplerLayoutBinding = {
			layoutBindingIndex++,
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			m_memoryManager->GetNumberTextures(),
			VK_SHADER_STAGE_FRAGMENT_BIT,
			nullptr
		};
		layoutBindings.emplace_back(samplerLayoutBinding);

		VkDescriptorSetLayoutCreateInfo descriptorLayoutInfo = {
			VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
			nullptr,
			0,
			layoutBindings.size(),
			layoutBindings.data()
		};
		m_descriptorSetLayouts.resize(1);
		VkResult result = vkCreateDescriptorSetLayout(Device::Get().m_device, &descriptorLayoutInfo, nullptr, m_descriptorSetLayouts.data());
		if (result != VK_SUCCESS)
		{
			Logger::Log("Could not create uniform buffer descriptor set layout.");
			return false;
		}

		std::vector<VkPushConstantRange> pushConstantRanges;
		/*
		VkPushConstantRange pushConstantRangeTransforms = {};
		pushConstantRangeTransforms.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		pushConstantRangeTransforms.offset = 0;
		pushConstantRangeTransforms.size = sizeof(ObjectData);
		pushConstantRanges.emplace_back(pushConstantRangeTransforms);
		*/

		VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {
			VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
			nullptr,
			0,
			m_descriptorSetLayouts.size(),
			m_descriptorSetLayouts.data(),
			pushConstantRanges.size(),
			pushConstantRanges.data()
		};
		result = vkCreatePipelineLayout(Device::Get().m_device, &pipelineLayoutCreateInfo, nullptr, &m_pipelineLayout);
		if (result != VK_SUCCESS)
		{
			Logger::Log("Could not create pipeline layout.");
			return false;
		}

		return true;
	}

	bool PipelineRasterization::CreateDescriptorPool()
	{
		std::vector<VkDescriptorPoolSize> descriptorPoolSizes;
		VkDescriptorPoolSize poolSizeViewProjection = {};
		poolSizeViewProjection.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSizeViewProjection.descriptorCount = 1; 
		descriptorPoolSizes.emplace_back(poolSizeViewProjection);

		VkDescriptorPoolSize poolSizeDynamicTransform = {};
		poolSizeDynamicTransform.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
		poolSizeDynamicTransform.descriptorCount = 1;
		descriptorPoolSizes.emplace_back(poolSizeDynamicTransform);

		VkDescriptorPoolSize storageBufferPoolSize = {};
		storageBufferPoolSize.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		storageBufferPoolSize.descriptorCount = m_scene->m_drawables.size();
		descriptorPoolSizes.emplace_back(storageBufferPoolSize);

		VkDescriptorPoolSize poolSizeTextureSampler = {};
		poolSizeTextureSampler.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		poolSizeTextureSampler.descriptorCount = m_memoryManager->GetNumberTextures();
		descriptorPoolSizes.emplace_back(poolSizeTextureSampler);

		VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = {
			VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
			nullptr,
			0,
			1, //TODO: max sets, make non constant, take number from CreatePipelineLayout
			descriptorPoolSizes.size(),
			descriptorPoolSizes.data()
		};
		VkResult result = vkCreateDescriptorPool(Device::Get().m_device, &descriptorPoolCreateInfo, nullptr, &m_descriptorPool);
		if (result != VK_SUCCESS)
		{
			Logger::Log("Could not create descriptor pool.");
			return false;
		}

		return true;
	}

	bool PipelineRasterization::CreateDescriptorSets()
	{
		VkDescriptorSetAllocateInfo allocInfo;
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.pNext = nullptr;
		allocInfo.descriptorPool = m_descriptorPool;
		allocInfo.descriptorSetCount = m_descriptorSetLayouts.size();
		allocInfo.pSetLayouts = m_descriptorSetLayouts.data();
		m_descriptorSets.resize(m_descriptorSetLayouts.size());
		VkResult result = vkAllocateDescriptorSets(Device::Get().m_device, &allocInfo, m_descriptorSets.data());
		if (result != VK_SUCCESS)
		{
			Logger::Log("Could not allocate descriptor set.");
			return false;
		}

		std::vector<VkDescriptorBufferInfo> materialDescBufferInfo;
		for (size_t i = 0; i < m_scene->m_drawables.size(); ++i)
		{
			materialDescBufferInfo.push_back({ m_scene->m_drawables[i].m_materialBuffer, 0, VK_WHOLE_SIZE });
		}

		std::vector<VkWriteDescriptorSet> descriptorSetWrites;
		uint32_t dstBinding = 0;

		//camera
		VkWriteDescriptorSet viewProjectionUBODescriptorSet;
		viewProjectionUBODescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		viewProjectionUBODescriptorSet.pNext = nullptr;
		viewProjectionUBODescriptorSet.dstSet = m_descriptorSets[0];
		viewProjectionUBODescriptorSet.descriptorCount = 1;
		viewProjectionUBODescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		viewProjectionUBODescriptorSet.pBufferInfo = m_camera->GetCameraDescriptor();
		viewProjectionUBODescriptorSet.dstArrayElement = 0;
		viewProjectionUBODescriptorSet.dstBinding = dstBinding++;
		descriptorSetWrites.emplace_back(viewProjectionUBODescriptorSet);

		//transform dyn ubo
		VkWriteDescriptorSet dynamicTransformUBO;
		dynamicTransformUBO.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		dynamicTransformUBO.pNext = nullptr;
		dynamicTransformUBO.dstSet = m_descriptorSets[0];
		dynamicTransformUBO.descriptorCount = 1;
		dynamicTransformUBO.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
		dynamicTransformUBO.pBufferInfo = &m_dynamicTransformBuffer.m_descriptorBufferInfo;
		dynamicTransformUBO.dstArrayElement = 0;
		dynamicTransformUBO.dstBinding = dstBinding++;
		descriptorSetWrites.emplace_back(dynamicTransformUBO);

		//materials
		VkWriteDescriptorSet materialDescriptorSet;
		materialDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		materialDescriptorSet.pNext = nullptr;
		materialDescriptorSet.dstSet = m_descriptorSets[0];
		materialDescriptorSet.descriptorCount = materialDescBufferInfo.size();
		materialDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		materialDescriptorSet.pBufferInfo = materialDescBufferInfo.data();
		materialDescriptorSet.dstArrayElement = 0;
		materialDescriptorSet.dstBinding = dstBinding++;
		descriptorSetWrites.emplace_back(materialDescriptorSet);

		//image sampler
		VkWriteDescriptorSet imageSamplerDescriptorSet;
		imageSamplerDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		imageSamplerDescriptorSet.pNext = nullptr;
		imageSamplerDescriptorSet.dstSet = m_descriptorSets[0];
		imageSamplerDescriptorSet.descriptorCount = m_memoryManager->GetNumberTextures();
		imageSamplerDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		imageSamplerDescriptorSet.pImageInfo = m_memoryManager->GetDescriptorImageInfo();
		imageSamplerDescriptorSet.dstArrayElement = 0;
		imageSamplerDescriptorSet.dstBinding = dstBinding++;
		descriptorSetWrites.emplace_back(imageSamplerDescriptorSet);

		vkUpdateDescriptorSets(Device::Get().m_device, descriptorSetWrites.size(), descriptorSetWrites.data(), 0, nullptr);

		return true;
	}
}