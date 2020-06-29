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

	struct RtPushConstant
	{
		glm::vec4 clearColor;
		glm::vec3 lightPosition;
		float     lightIntensity;
		int       numberOfSamples;
	};

	struct ShaderBindingTableEntry
	{
		uint8_t shaderGroupHandle[16]; //Vulkan shader group size
		uint32_t geometryID;
		uint8_t padding[12]; //from size 20 to 32 bytes
		uint8_t padding2[32];
	};

	class PipelineRaytracing : public Pipeline
	{
	public:
		void Init(VkPhysicalDevice& device, DeviceMemoryManager& memoryManager, VkRenderPass& renderPass, VkExtent2D windowExtent) override;
		void Tick(VkCommandBuffer& commandBuffer) override;
		void Fini();

		void FillRenderpassInfo(Renderpass* renderpass) override;
		void RecreateOutput(VkExtent2D& windowExtent);
		void SetCamera(Camera* camera);
		void SetScene(Scene* scene);
		void SetRaytracingProperties(VkPhysicalDeviceRayTracingPropertiesNV* raytracingProperties);

		bool UpdateTransformations();

		//TODO: move
		VkImage GetStorageImage();

	protected:
		VkPhysicalDeviceRayTracingPropertiesNV* m_raytracingProperties;

		Scene* m_scene;
		Camera* m_camera;

		//map of drawable handles and vectors of instance handles
		std::unordered_map<uint32_t, std::vector<uint32_t>> m_dynamicDrawableInstances;
		std::vector<uint32_t> m_staticDrawableInstances;

		//BLAS
		bool ConvertToGeometryNV(std::vector<VkGeometryNV>& target, uint32_t drawableHandle);
		bool ConvertToGeometryNV(std::vector<VkGeometryNV>& target, uint32_t drawableHandle, uint32_t instanceHandle);
		bool PrepareDrawableInstances();
		bool CreateBLAS();
		bool UpdateBLASInstances();
		std::vector<std::vector<VkGeometryNV>> m_rtGeometries;
		std::vector<BLAS> m_blasVector;

		//TLAS
		bool CreateTLAS();
		TLAS m_tlas;
		std::vector<BLASInstance> m_blasInstances;
		VkBuffer m_instanceBuffer;
		VkDeviceMemory m_instanceBufferMemory;

		//TODO: integrate with simple scene graph, DrawableInstance to NodeDrawable
		//scene description
		bool CreateSceneInformationBuffer();
		
		VkBuffer m_sceneBuffer;
		VkDeviceMemory m_sceneBufferMemory;
		VkDescriptorBufferInfo m_sceneBufferDescriptor;

		//storage image
		VkImage m_storageImage;
		VkImageView m_storageImageView;
		VkDeviceMemory m_storageImageMemory;
		bool CreateStorageImage();
		bool CleanupStorageImage();

		//shader binding table
		bool CreateShaderBindingTable();
		VkBuffer m_shaderBindingTable;
		VkDeviceMemory m_shaderBindingTableMemory;
		VkDeviceSize m_shaderBindingTableStride = 64;
		std::vector<uint32_t> m_shaderBindingGeometryIDs;

		RtPushConstant m_rtPushConstants;


		//overrides
		//---------------------------------------

		void DefineVertices() override;

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
		bool CreateDescriptorSets() override;
		//---------------------------------------

	};
}