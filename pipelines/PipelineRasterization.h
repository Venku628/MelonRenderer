#pragma once
#include "Pipeline.h"

namespace MelonRenderer
{
	class PipelineRasterization : public Pipeline
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
		bool CleanupSwapchain() override;

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



		void InitCam();

		//drawables, pipeline specific
		//---------------------------------------
		std::vector<Drawable> m_drawables;
		bool CreateDrawableBuffers(Drawable& drawable);
		//---------------------------------------

		//everything that should be partially moved to the memoryManager
		//---------------------------------------

		//TODO: depth buffer class?
		VkImage m_depthBuffer;
		VkDeviceMemory m_depthBufferMemory;
		VkImageView m_depthBufferView;
		VkFormat m_depthBufferFormat;

		bool CreateDepthBuffer();

		//uniform buffer class? buffers should also be allocated in bulk 
		mat4 m_modelViewProjection;
		VkBuffer m_uniformBuffer;
		VkDeviceMemory m_uniformBufferMemory;

		bool CreateUniformBufferMVP();
		//---------------------------------------
	};
}