#pragma once
#include "Pipeline.h"
#include "imgui.h"

namespace MelonRenderer
{
	class PipelineImGui : public Pipeline
	{
	public:
		void Init(VkPhysicalDevice& device, DeviceMemoryManager& memoryManager, OutputSurface outputSurface, VkExtent2D windowExtent) override;
		void Tick(float timeDelta) override;
		void RecreateSwapchain(VkExtent2D windowExtent) override;

		void Fini() override;

	protected:
		//virtual void     = 0; in pipeline base
		void DefineVertices() override;

		bool CreateSwapchain(VkPhysicalDevice& device) override;

		const uint32_t m_numberOfSamples = 1;

		//---------------------------------------
		bool AquireNextImage() override;
		//---------------------------------------

		//Renderpass
		//---------------------------------------
		bool CreateRenderPass() override;
		//---------------------------------------


		//shader modules
		//---------------------------------------
		bool CreateShaderModules() override;
		//---------------------------------------


		//frame buffers
		//---------------------------------------
		bool CreateFramebuffers() override;
		//---------------------------------------

		//---------------------------------------
		bool CreateGraphicsPipeline() override;
		//---------------------------------------

		bool Draw(float timeDelta) override;


		//---------------------------------------
		bool CreatePipelineLayout() override;
		bool CreateDescriptorPool() override;
		bool CreateDescriptorSet() override;
		//---------------------------------------


		bool CreateFontSampler();
		VkSampler m_fontSampler;

		VkImageView m_fontImageView;
	};
}