#pragma once
#include "Pipeline.h"

namespace MelonRenderer
{
	class PipelineImGui : public Pipeline
	{
	public:
		void Init(VkPhysicalDevice& physicalDevice, DeviceMemoryManager& memoryManager, VkRenderPass& renderPass, VkExtent2D windowExtent) override;
		void Tick(VkCommandBuffer& commanduffer) override;
		void Fini();

		void FillRenderpassInfo(Renderpass* renderpass) override;
		void RecreateOutput(VkExtent2D& windowExtent);

	protected:
		//virtual void     = 0; in pipeline base
		void DefineVertices() override;

		//shader modules
		//---------------------------------------
		bool CreateShaderModules() override;
		//---------------------------------------

		//---------------------------------------
		bool CreateGraphicsPipeline() override;
		//---------------------------------------

		bool Draw(VkCommandBuffer& commandBuffer) override;


		//---------------------------------------
		bool CreatePipelineLayout() override;
		bool CreateDescriptorPool() override;
		bool CreateDescriptorSets() override;
		//---------------------------------------

		VkFence m_fence;

		bool CreateFontSampler();
		VkSampler m_fontSampler;

		bool CreateFence();

		bool CreateRenderState(VkCommandBuffer& commandBuffer);
		bool CreateImGuiDrawDataBuffer();
		bool CreateFontTexture();
		VkBuffer m_vertexBuffer;
		VkBuffer m_indexBuffer;
		VkDeviceMemory m_vertexBufferMemory;
		VkDeviceMemory m_indexBufferMemory;
		VkDeviceSize m_vertexBufferSize;
		VkDeviceSize m_indexBufferSize;

		uint32_t m_subpassNumber = 0;

		VkImage m_fontImage;
		VkDeviceMemory m_fontImageMemory;
		VkImageView m_fontImageView;

		bool debugimgui = false;
	};
}