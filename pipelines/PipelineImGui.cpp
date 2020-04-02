#include "PipelineImGui.h"
#include "../imgui/imgui.h"

namespace MelonRenderer
{
	void PipelineImGui::Init(VkPhysicalDevice& physicalDevice, DeviceMemoryManager& memoryManager, VkRenderPass& renderPass, VkExtent2D windowExtent)
	{
		m_physicalDevice = &physicalDevice;
		m_memoryManager = &memoryManager;
		m_renderPass = &renderPass;
		m_extent = windowExtent;

		DefineVertices();

		CreateFontTexture();
		CreateFence();

		CreatePipelineLayout();
		CreateDescriptorPool();
		CreateDescriptorSet();

		CreateShaderModules();

		CreateGraphicsPipeline();
	}

	void PipelineImGui::Tick(VkCommandBuffer& commandBuffer)
	{
		Draw(commandBuffer);
	}

	void PipelineImGui::Fini()
	{
		
	}

	void PipelineImGui::RecreateOutput(VkExtent2D& windowExtent)
	{
		m_extent = windowExtent;
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

		VkPipelineRasterizationStateCreateInfo pipelineRasterizationInfo = {};
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

		VkPipelineColorBlendStateCreateInfo pipelineColorBlendInfo = {};
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

		VkPipelineDepthStencilStateCreateInfo pipelineDepthStencilInfo = {};
		pipelineDepthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		pipelineDepthStencilInfo.pNext = nullptr;
		pipelineDepthStencilInfo.flags = 0;
		pipelineDepthStencilInfo.depthTestEnable = VK_FALSE;
		pipelineDepthStencilInfo.depthWriteEnable = VK_FALSE;

		//setup for no multisampling for now, imgui does not profit a lot from MSAA according to it´s documentation
		VkPipelineMultisampleStateCreateInfo pipelineMultisampleInfo = {};
		pipelineMultisampleInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		pipelineMultisampleInfo.pNext = nullptr;
		pipelineMultisampleInfo.flags = 0;
		pipelineMultisampleInfo.pSampleMask = NULL;
		pipelineMultisampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT; 
		pipelineMultisampleInfo.sampleShadingEnable = VK_FALSE;
		pipelineMultisampleInfo.alphaToCoverageEnable = VK_FALSE;
		pipelineMultisampleInfo.alphaToOneEnable = VK_FALSE;
		pipelineMultisampleInfo.minSampleShading = 0.0;

		VkGraphicsPipelineCreateInfo pipeline = {};
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
		pipeline.renderPass = *m_renderPass;
		pipeline.subpass = 0; //TODO: make parameter in case of more pipelines

		VkResult result = vkCreateGraphicsPipelines(Device::Get().m_device, VK_NULL_HANDLE, 1, &pipeline, nullptr, &m_pipeline);
		if (result != VK_SUCCESS)
		{
			Logger::Log("Could not create graphics pipeline.");
			return false;
		}

		return true;
	}

	bool PipelineImGui::Draw(VkCommandBuffer& commandBuffer)
	{
		if(CreateImGuiDrawDataBuffer())
		{
			CreateRenderState(commandBuffer);

			ImDrawData* imguiDrawData = ImGui::GetDrawData();
			// Will project scissor/clipping rectangles into framebuffer space
			ImVec2 clipOff = imguiDrawData->DisplayPos;         // (0,0) unless using multi-viewports
			ImVec2 clipScale = imguiDrawData->FramebufferScale; // (1,1) unless using retina display which are often (2,2)

			// Render command lists
			// (Because we merged all buffers into a single one, we maintain our own offset into them)
			int vertexOffset = 0;
			int indexOffset = 0;
			for (int i = 0; i < imguiDrawData->CmdListsCount; i++)
			{
				const ImDrawList* imguiCmdList = imguiDrawData->CmdLists[i];
				for (int cmd_i = 0; cmd_i < imguiCmdList->CmdBuffer.Size; cmd_i++)
				{
					const ImDrawCmd* imguiDrawCommand = &imguiCmdList->CmdBuffer[cmd_i];
					
					// Project scissor/clipping rectangles into framebuffer space
					ImVec4 clipRect;
					clipRect.x = (imguiDrawCommand->ClipRect.x - clipOff.x) * clipScale.x;
					clipRect.y = (imguiDrawCommand->ClipRect.y - clipOff.y) * clipScale.y;
					clipRect.z = (imguiDrawCommand->ClipRect.z - clipOff.x) * clipScale.x;
					clipRect.w = (imguiDrawCommand->ClipRect.w - clipOff.y) * clipScale.y;

					if (clipRect.x < m_extent.width && clipRect.y < m_extent.height && clipRect.z >= 0.0f && clipRect.w >= 0.0f)
					{
						// Negative offsets are illegal for vkCmdSetScissor
						if (clipRect.x < 0.0f)
							clipRect.x = 0.0f;
						if (clipRect.y < 0.0f)
							clipRect.y = 0.0f;

						// Apply scissor/clipping rectangle
						VkRect2D scissor;
						scissor.offset.x = (int32_t)(clipRect.x);
						scissor.offset.y = (int32_t)(clipRect.y);
						scissor.extent.width = (uint32_t)(clipRect.z - clipRect.x);
						scissor.extent.height = (uint32_t)(clipRect.w - clipRect.y);
						vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

						// Draw
						vkCmdDrawIndexed(commandBuffer, imguiDrawCommand->ElemCount, 1, imguiDrawCommand->IdxOffset + indexOffset,
							imguiDrawCommand->VtxOffset + vertexOffset, 0);

					}
					
				}
				indexOffset += imguiCmdList->IdxBuffer.Size;
				vertexOffset += imguiCmdList->VtxBuffer.Size;
			}
		}
		return true;
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

		VkDescriptorSetLayoutCreateInfo descriptorLayoutInfo = {};
		descriptorLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		descriptorLayoutInfo.bindingCount = 1;
		descriptorLayoutInfo.pBindings = binding;
		m_descriptorSetLayouts.resize(1);
		VkResult result = vkCreateDescriptorSetLayout(Device::Get().m_device, &descriptorLayoutInfo, nullptr, m_descriptorSetLayouts.data());
		if (result != VK_SUCCESS)
		{
			Logger::Log("Could not create descriptor set layout.");
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

	bool PipelineImGui::CreateFence()
	{
		VkFenceCreateInfo fenceInfo = {};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.pNext = nullptr;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
		vkCreateFence(Device::Get().m_device, &fenceInfo, nullptr, &m_fence);

		return true;
	}

	bool PipelineImGui::CreateRenderState(VkCommandBuffer& commandBuffer)
	{
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineLayout, 0, m_descriptorSets.size(),
			m_descriptorSets.data(), 0, nullptr);

		VkBuffer vertex_buffers[1] = { m_vertexBuffer };
		VkDeviceSize vertex_offset[1] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertex_buffers, vertex_offset);
		vkCmdBindIndexBuffer(commandBuffer, m_indexBuffer, 0, sizeof(ImDrawIdx) == 2 ? VK_INDEX_TYPE_UINT16 : VK_INDEX_TYPE_UINT32);

		m_viewport.width = (float)m_extent.width;
		m_viewport.height = (float)m_extent.height;
		m_viewport.minDepth = (float)0.0f;
		m_viewport.maxDepth = (float)1.0f;
		m_viewport.x = 0;
		m_viewport.y = 0;
		vkCmdSetViewport(commandBuffer, 0, 1, &m_viewport);

		float scale[2];
		scale[0] = 2.0f / m_extent.width;
		scale[1] = 2.0f / m_extent.height;
		float translate[2];
		translate[0] = -1.0f - 0 * scale[0]; //for single viewpoint apps
		translate[1] = -1.0f - 0 * scale[1];
		vkCmdPushConstants(commandBuffer, m_pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, sizeof(float) * 0, sizeof(float) * 2, scale);
		vkCmdPushConstants(commandBuffer, m_pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, sizeof(float) * 2, sizeof(float) * 2, translate);

		return true;
	}

	bool PipelineImGui::CreateImGuiDrawDataBuffer()
	{
		ImDrawData* imguiDrawData = ImGui::GetDrawData();

		// Avoid rendering when minimized, scale coordinates for retina displays (screen coordinates != framebuffer coordinates)
		int fb_width = (int)(imguiDrawData->DisplaySize.x * imguiDrawData->FramebufferScale.x);
		int fb_height = (int)(imguiDrawData->DisplaySize.y * imguiDrawData->FramebufferScale.y);
		if (fb_width <= 0 || fb_height <= 0 || imguiDrawData->TotalVtxCount == 0)
			return false;

		VkResult result;

		// Create or resize the vertex/index buffers
		size_t vertexBufferSize = imguiDrawData->TotalVtxCount * sizeof(ImDrawVert);
		size_t indexBufferSize = imguiDrawData->TotalIdxCount * sizeof(ImDrawIdx);
		if (m_vertexBuffer == VK_NULL_HANDLE || m_vertexBufferSize < vertexBufferSize)
		{
			if (m_vertexBuffer != VK_NULL_HANDLE)
				vkDestroyBuffer(Device::Get().m_device, m_vertexBuffer, nullptr);
			if (m_vertexBufferMemory != VK_NULL_HANDLE)
				vkFreeMemory(Device::Get().m_device, m_vertexBufferMemory, nullptr);

			VkDeviceSize vertexBufferSizeAligned = ((vertexBufferSize - 1) / 256 + 1) * 256;
			m_memoryManager->CreateBuffer(vertexBufferSizeAligned, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
				m_vertexBuffer, m_vertexBufferMemory);
			m_vertexBufferSize = vertexBufferSize;
		}

		if (m_indexBuffer == VK_NULL_HANDLE || m_indexBufferSize < indexBufferSize)
		{
			if (m_indexBuffer != VK_NULL_HANDLE)
				vkDestroyBuffer(Device::Get().m_device, m_indexBuffer, nullptr);
			if (m_indexBufferMemory != VK_NULL_HANDLE)
				vkFreeMemory(Device::Get().m_device, m_indexBufferMemory, nullptr);

			VkDeviceSize indexBufferSizeAligned = ((indexBufferSize - 1) / 256 + 1) * 256;
			m_memoryManager->CreateBuffer(indexBufferSizeAligned, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
				m_indexBuffer, m_indexBufferMemory);
			m_indexBufferSize = indexBufferSize;
		}
			
		// Upload vertex/index data into a single contiguous GPU buffer
		ImDrawVert* vertexData = NULL;
		ImDrawIdx* indexData = NULL;
		result = vkMapMemory(Device::Get().m_device, m_vertexBufferMemory, 0, vertexBufferSize, 0, (void**)(&vertexData));
		if (result != VK_SUCCESS)
		{
			Logger::Log("Could not map memory for imgui vertex buffer.");
			return false;
		}
		result = vkMapMemory(Device::Get().m_device, m_indexBufferMemory, 0, indexBufferSize, 0, (void**)(&indexData));
		if (result != VK_SUCCESS)
		{
			Logger::Log("Could not map memory for imgui index buffer.");
			return false;
		}

		for (int i = 0; i < imguiDrawData->CmdListsCount; i++)
		{
			const ImDrawList* imguiCmdList = imguiDrawData->CmdLists[i];
			memcpy(vertexData, imguiCmdList->VtxBuffer.Data, imguiCmdList->VtxBuffer.Size * sizeof(ImDrawVert));
			memcpy(indexData, imguiCmdList->IdxBuffer.Data, imguiCmdList->IdxBuffer.Size * sizeof(ImDrawIdx));
			vertexData += imguiCmdList->VtxBuffer.Size;
			indexData += imguiCmdList->IdxBuffer.Size;
		}
		VkMappedMemoryRange range[2] = {};
		range[0].sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
		range[0].memory = m_vertexBufferMemory;
		range[0].size = VK_WHOLE_SIZE;
		range[1].sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
		range[1].memory = m_indexBufferMemory;
		range[1].size = VK_WHOLE_SIZE;
		result = vkFlushMappedMemoryRanges(Device::Get().m_device, 2, range);
		if (result != VK_SUCCESS)
		{
			Logger::Log("Could not map memory for imgui index buffer.");
			return false;
		}
		vkUnmapMemory(Device::Get().m_device, m_vertexBufferMemory);
		vkUnmapMemory(Device::Get().m_device, m_indexBufferMemory);

		return true;
	}

	bool PipelineImGui::CreateFontTexture()
	{
		int width, height;
		unsigned char* pixelData;
		ImGui::GetIO().Fonts->GetTexDataAsRGBA32(&pixelData, &width, &height);

		if (!m_memoryManager->CreateTextureImage(m_fontImage, m_fontImageMemory, pixelData, width, height))
		{
			Logger::Log("Could not create texture image and memory.");
			return false;
		}

		if (!m_memoryManager->CreateImageView(m_fontImageView, m_fontImage))
		{
			Logger::Log("Could not create texture view.");
			return false;
		}

		ImGui::GetIO().Fonts->TexID = (ImTextureID)(intptr_t)m_fontImage;

		return true;
	}
}