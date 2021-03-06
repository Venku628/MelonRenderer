#pragma once

#include "../Basics.h"
#include "../Drawable.h"
#include "../Shader.h"
#include "../DeviceMemoryManager.h"
#include "../Renderpass.h"

#include <vector>

namespace MelonRenderer
{
	class Pipeline
	{
	public:
		virtual void Init(VkPhysicalDevice& physicalDevice, DeviceMemoryManager& memoryManager, VkRenderPass& renderPass, VkExtent2D windowExtent) = 0;
		virtual void Tick(VkCommandBuffer& commanduffer) = 0;

		virtual void FillRenderpassInfo(Renderpass* renderpass) = 0;

	protected:
		virtual void DefineVertices() = 0;

		//TODO: make this variable
		const uint32_t m_numberOfSamples = 1;
		VkExtent2D m_extent;

		std::vector<VkAttachmentReference> m_attachmentReferences;

		//TODO: move
		VkRenderPass* m_renderpass;

		//shader modules
		//---------------------------------------
		//this struct holds the shader modules
		std::vector<VkPipelineShaderStageCreateInfo> m_shaderStagesV;
		//TODO: destroy the shader stages after they are used to create the graphics pipeline

		bool CreateShaderModule(const std::vector<char>& code, VkShaderModule& shaderModule);
		virtual bool CreateShaderModules() = 0;
		//---------------------------------------

		//vertex buffer
		//---------------------------------------
		std::vector<VkVertexInputAttributeDescription> m_vertexInputAttributes;
		std::vector<VkVertexInputBindingDescription> m_vertexInputBindings;
		//---------------------------------------

		//---------------------------------------
		VkPipeline m_pipeline;
		virtual bool CreateGraphicsPipeline() = 0;
		//---------------------------------------

		virtual bool Draw(VkCommandBuffer& commandBuffer) = 0;

		//TODO: move
		VkViewport m_viewport;
		VkRect2D m_scissorRect2D;

		//---------------------------------------
		VkDescriptorPool m_descriptorPool;
		std::vector<VkDescriptorSet> m_descriptorSets;
		std::vector<VkDescriptorSetLayout> m_descriptorSetLayouts;
		VkPipelineLayout m_pipelineLayout;
		virtual bool CreatePipelineLayout() = 0;
		virtual bool CreateDescriptorPool() = 0;
		virtual bool CreateDescriptorSets() = 0;
		//---------------------------------------

		DeviceMemoryManager* m_memoryManager;
	};
}