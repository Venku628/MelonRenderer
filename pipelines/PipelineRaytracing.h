#pragma once

#include "Pipeline.h"
#include "../simple_scene_graph/Scene.h"
#include "../imgui/imgui.h"

namespace MelonRenderer
{
	// Bottom-level acceleration structure
	struct BLAS
	{
		VkAccelerationStructureNV m_accelerationStructure = VK_NULL_HANDLE;
		VkDeviceMemory m_accelerationStructureMemory = VK_NULL_HANDLE;
		VkAccelerationStructureInfoNV m_accelerationStructureInfo = {
		VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_INFO_NV,
		nullptr,
		VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_NV,
		VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_NV,
		1,
		1,
		nullptr}; 
		uint64_t m_handle = 0;
	};

	//from vulkan spec 
	struct BLASInstance
	{
		glm::mat3x4 m_transform;
		uint32_t m_instanceId : 24;
		uint32_t m_mask : 8;
		uint32_t m_instanceOffset : 24;
		uint32_t m_flags : 8;
		uint64_t m_accelerationStructureHandle;
	};

	// Top-level acceleration structure
	struct TLAS
	{
		VkAccelerationStructureNV m_accelerationStructure = VK_NULL_HANDLE;
		VkDeviceMemory m_accelerationStructureMemory = VK_NULL_HANDLE;
		VkAccelerationStructureInfoNV m_accelerationStructureInfo = {
		VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_INFO_NV,
		nullptr,
		VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_NV,
		VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_NV,
		1,
		1,
		nullptr };
		uint64_t m_handle = 0;
	};

	//TODO: integrate into Drawable node
	struct DrawableInstance
	{
		uint32_t m_drawableIndex;
		uint32_t m_textureOffset;
		mat4 m_transformation;
		mat4 m_transformationInverseTranspose;
	};

	struct RtPushConstant
	{
		glm::vec4 clearColor;
		glm::vec3 lightPosition;
		float     lightIntensity;
		int       lightType;
	};

	class PipelineRaytracing : public Pipeline
	{
	public:
		void Init(VkPhysicalDevice& device, DeviceMemoryManager& memoryManager, VkRenderPass& renderPass, VkExtent2D windowExtent) override;
		void Tick(VkCommandBuffer& commandBuffer) override;
		void Fini() override;

		void RecreateOutput(VkExtent2D& windowExtent);
		void SetCamera(Camera* camera);
		void SetScene(Scene* scene);
		void SetRaytracingProperties(VkPhysicalDeviceRayTracingPropertiesNV* raytracingProperties);

		//TODO: move
		VkImage GetStorageImage();

	protected:
		VkPhysicalDeviceRayTracingPropertiesNV* m_raytracingProperties;

		Scene* m_scene;
		Camera* m_camera;
		std::vector<Drawable*> m_drawables;

		//BLAS
		bool ConvertToGeometryNV(const Drawable& drawable, uint32_t transformMatIndex);
		bool ConvertDrawables();
		bool CreateBLAS();
		//TODO: expand for multiple BLAS
		std::vector<std::vector<VkGeometryNV>> m_rtGeometries;
		std::vector<BLAS> m_blasVector;

		//TLAS
		bool CreateTLAS();
		TLAS m_tlas;
		std::vector<mat3x4> m_instanceTranforms; // make this ObjInstance with objIndex and transform information
		std::vector<BLASInstance> m_blasInstances;
		VkBuffer m_instanceBuffer;
		VkDeviceMemory m_instanceBufferMemory;

		//TODO: integrate with simple scene graph, DrawableInstance to NodeDrawable
		//scene description
		bool CreateSceneInformationBuffer();
		VkBuffer m_sceneBuffer;
		VkDeviceMemory m_sceneBufferMemory;
		std::vector<DrawableInstance> m_drawableInstances;
		VkDescriptorBufferInfo m_sceneBufferDescriptor;

		//storage image
		VkImage m_storageImage;
		VkImageView m_storageImageView;
		VkDeviceMemory m_storageImageMemory;
		bool CreateStorageImage();

		//shader binding table
		bool CreateShaderBindingTable();
		VkBuffer m_shaderBindingTable;
		VkDeviceMemory m_shaderBindingTableMemory;

		RtPushConstant m_rtPushConstants;


		//overrides
		//---------------------------------------

		void DefineVertices() override;

		const uint32_t m_numberOfSamples = 1;

		//shader modules
		//---------------------------------------
		bool CreateShaderModules() override;
		std::vector<VkRayTracingShaderGroupCreateInfoNV> m_rtShaderGroups;
		//---------------------------------------

		//---------------------------------------
		bool CreateGraphicsPipeline() override;
		//---------------------------------------

		bool Draw(VkCommandBuffer& commandBuffer) override;


		//---------------------------------------
		bool CreatePipelineLayout() override;
		bool CreateDescriptorPool() override;
		bool CreateDescriptorSet() override;
		//---------------------------------------

	};
}