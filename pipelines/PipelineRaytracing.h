#pragma once

#include "Pipeline.h"
#include "../simple_scene_graph/Scene.h"

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

	struct BLASInstance
	{
		uint64_t m_accelerationStructureHandle;
		uint32_t m_instanceId;
		uint32_t m_hitGroupId;
		uint32_t m_flags;
		glm::mat3x4 m_transform;
	};

	// Top-level acceleration structure
	struct TLAS
	{
		VkAccelerationStructureNV m_accelerationStructure;
		VkDeviceMemory m_accelerationStructureMemory;
		VkAccelerationStructureInfoNV m_accelerationStructureInfo = {
		VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_INFO_NV,
		nullptr,
		VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_NV,
		VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_NV,
		1,
		1,
		nullptr };
		uint64_t m_handle;
	};

	struct RtPushConstant
	{
		glm::vec4 clearColor;
		glm::vec3 lightPosition;
		float     lightIntensity;
		int       lightType;
	} m_rtPushConstants;

	class PipelineRaytracing : public Pipeline
	{
	public:
		void Init(VkPhysicalDevice& device, DeviceMemoryManager& memoryManager, VkRenderPass& renderPass, VkExtent2D windowExtent) override;
		void Tick(VkCommandBuffer& commandBuffer) override;
		void Fini() override;

		void FillAttachments(std::vector<VkImageView>* attachments);
		void RecreateOutput(VkExtent2D& windowExtent);
		void SetCamera(Camera* camera);
		void SetScene(Scene* scene);
		void SetRaytracingProperties(VkPhysicalDeviceRayTracingPropertiesNV* raytracingProperties);

	protected:
		VkPhysicalDeviceRayTracingPropertiesNV* m_raytracingProperties;
		Scene* m_scene;

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
		std::vector<mat3x4> m_instanceTranforms;
		std::vector<BLASInstance> m_blasInstances;

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