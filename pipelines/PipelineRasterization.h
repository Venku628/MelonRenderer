#pragma once
#include "Pipeline.h"
#include "../Camera.h"
#include "../simple_scene_graph/Scene.h"

namespace MelonRenderer
{
	struct SceneInfo
	{
		vec3 lightPosition;
		float lightIntensity;
	};


	class PipelineRasterization : public Pipeline
	{
	public:
		void Init(VkPhysicalDevice& device, DeviceMemoryManager& memoryManager, VkRenderPass& renderPass, VkExtent2D windowExtent) override;
		void Tick(VkCommandBuffer& commanduffer) override;
		void Fini();

		void FillRenderpassInfo(Renderpass* renderpass) override;
		void FillAttachmentInfo(Swapchain* swapchain); //TODO: implement for every pipeline?
		void RecreateOutput(VkExtent2D& windowExtent);
		void SetCamera(Camera* camera);
		void SetScene(Scene* scene);

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

		bool CreateDynamicTransformBuffer();
		bool UpdateDynamicTransformBuffer();
		DynamicUniformBuffer m_dynamicTransformBuffer;

		bool Draw(VkCommandBuffer& commandBuffer) override;


		//---------------------------------------
		bool CreatePipelineLayout() override;
		bool CreateDescriptorPool() override;
		bool CreateDescriptorSets() override;
		//---------------------------------------

		Camera* m_camera;
		Scene* m_scene;

		SceneInfo m_pushConstants;


		VkAttachmentReference m_depthAttachmentReference;

		//everything that should be partially moved to the memoryManager
		//---------------------------------------

		//TODO: depth buffer class?
		VkImage m_depthBuffer;
		VkDeviceMemory m_depthBufferMemory;
		VkImageView m_depthBufferView;
		VkFormat m_depthBufferFormat = VK_FORMAT_D32_SFLOAT; 
		bool CreateDepthBuffer();
		bool CleanupDepthBuffer();
		//---------------------------------------
	};
}