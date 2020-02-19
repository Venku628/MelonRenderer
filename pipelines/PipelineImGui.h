#pragma once
#include "Pipeline.h"

namespace MelonRenderer
{
	class PipelineImGui : public Pipeline
	{
	public:
		void Init(VkPhysicalDevice& physicalDevice, DeviceMemoryManager& memoryManager, VkRenderPass& renderPass, VkExtent2D windowExtent) override;
		void Tick(VkCommandBuffer& commanduffer, float timeDelta) override;
		void Fini() override;

		void RecreateOutput(VkExtent2D& windowExtent);

	protected:
		//virtual void     = 0; in pipeline base
		void DefineVertices() override;

		const uint32_t m_numberOfSamples = 1;

		//shader modules
		//---------------------------------------
		bool CreateShaderModules() override;
		//---------------------------------------

		//---------------------------------------
		bool CreateGraphicsPipeline() override;
		//---------------------------------------

		bool Draw(VkCommandBuffer& commandBuffer, float timeDelta) override;


		//---------------------------------------
		bool CreatePipelineLayout() override;
		bool CreateDescriptorPool() override;
		bool CreateDescriptorSet() override;
		//---------------------------------------


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

		VkImage m_fontImage;
		VkDeviceMemory m_fontImageMemory;
		VkImageView m_fontImageView;

		bool debugimgui = false;
	};
}