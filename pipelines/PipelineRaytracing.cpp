#include "PipelineRaytracing.h"

namespace MelonRenderer
{
	void PipelineRaytracing::Init(VkPhysicalDevice& device, DeviceMemoryManager& memoryManager, VkRenderPass& renderPass, VkExtent2D windowExtent)
	{
		m_physicalDevice = &device;
		m_memoryManager = &memoryManager;
		m_renderPass = &renderPass;
		m_extent = windowExtent;

		DefineVertices();

		ConvertDrawables();
		CreateBLAS();

		CreateTLAS();

		CreateStorageImage();

		CreatePipelineLayout();
		CreateDescriptorPool();
		CreateDescriptorSet();

		CreateShaderModules();
		CreateGraphicsPipeline();
		CreateShaderBindingTable();
	}

	void PipelineRaytracing::Tick(VkCommandBuffer& commandBuffer)
	{
		Draw(commandBuffer);
	}

	void PipelineRaytracing::Fini()
	{
	}

	void PipelineRaytracing::RecreateOutput(VkExtent2D& windowExtent)
	{
	}

	void PipelineRaytracing::SetCamera(Camera* camera)
	{
	}

	void PipelineRaytracing::SetScene(Scene* scene)
	{
		m_scene = scene;
	}

	void PipelineRaytracing::SetRaytracingProperties(VkPhysicalDeviceRayTracingPropertiesNV* raytracingProperties)
	{
		m_raytracingProperties = raytracingProperties;
	}

	VkImage PipelineRaytracing::GetStorageImage()
	{
		return m_storageImage;
	}

	bool PipelineRaytracing::ConvertToGeometryNV(const Drawable& drawable, uint32_t transformMatIndex)
	{
		VkGeometryTrianglesNV triangles = {};
		triangles.sType = VK_STRUCTURE_TYPE_GEOMETRY_TRIANGLES_NV;
		triangles.pNext = nullptr;
		triangles.vertexData = drawable.m_vertexBuffer;
		triangles.vertexOffset = 0;
		triangles.vertexCount = drawable.m_vertexCount;
		triangles.vertexStride = sizeof(Vertex);
		triangles.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT; //are the vertices defined correctly for this struct?
		triangles.indexData = drawable.m_indexBuffer;
		triangles.indexOffset = 0;
		triangles.indexCount = drawable.m_indexCount;
		triangles.indexType = VK_INDEX_TYPE_UINT32;
		//TODO: as soon as we have more than one object in a BLAS this should be uncommented
		/*
		triangles.transformData = m_memoryManager->GetDynamicTransformDescriptor()->buffer;
		triangles.transformOffset = transformMatIndex * m_memoryManager->GetDynamicUBOAlignment();
		*/
	
		VkGeometryDataNV geoData = {};
		geoData.triangles = triangles;
		geoData.aabbs = {};
		geoData.aabbs.sType = VK_STRUCTURE_TYPE_GEOMETRY_AABB_NV;

		VkGeometryNV geometry;
		geometry.sType = VK_STRUCTURE_TYPE_GEOMETRY_NV;
		geometry.pNext = nullptr;
		geometry.flags = VK_GEOMETRY_OPAQUE_BIT_NV;
		geometry.geometry = geoData;
		geometry.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_NV;

		//TODO: for now, one BLAS per object, later combine objects into one BLAS
		m_rtGeometries.push_back({ geometry });

		return true;
	}

	bool PipelineRaytracing::ConvertDrawables()
	{
		auto drawables = m_scene->SearchForDynamicDrawables();

		for (auto drawable : drawables)
		{
			ConvertToGeometryNV(*drawable, 0);
		}

		return true;
	}

	bool PipelineRaytracing::CreateBLAS()
	{
		m_blasVector.resize(m_rtGeometries.size());

		VkResult result;
		VkDeviceSize maxScratchSize = 0;

		//TODO: the nvidia sample suggests reusing memory to create the blas on the gpu

		for (int i = 0; i < m_blasVector.size(); i++)
		{
			BLAS& blas = m_blasVector[i];

			blas.m_accelerationStructureInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_NV;
			blas.m_accelerationStructureInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_NV; //redundant
			blas.m_accelerationStructureInfo.geometryCount = m_rtGeometries[i].size();
			blas.m_accelerationStructureInfo.pGeometries = m_rtGeometries[i].data();
			blas.m_accelerationStructureInfo.instanceCount = 0;

			VkAccelerationStructureCreateInfoNV accelerationStructureCreateInfo;
			accelerationStructureCreateInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_NV;
			accelerationStructureCreateInfo.pNext = nullptr;
			accelerationStructureCreateInfo.compactedSize = 0;
			accelerationStructureCreateInfo.info = blas.m_accelerationStructureInfo;

			result = vkCreateAccelerationStructureNV(Device::Get().m_device, &accelerationStructureCreateInfo, nullptr, &blas.m_accelerationStructure);
			if (result != VK_SUCCESS)
			{
				Logger::Log("Could not create bottom level acceleration structure.");
				return false;
			}

			VkAccelerationStructureMemoryRequirementsInfoNV memoryRequirementsInfo;
			memoryRequirementsInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_INFO_NV;
			memoryRequirementsInfo.pNext = nullptr;
			memoryRequirementsInfo.type = VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_OBJECT_NV;
			memoryRequirementsInfo.accelerationStructure = blas.m_accelerationStructure;

			VkMemoryRequirements2 memoryRequirements2 = {};
			vkGetAccelerationStructureMemoryRequirementsNV(Device::Get().m_device, &memoryRequirementsInfo, &memoryRequirements2);

			VkMemoryAllocateInfo memoryAllocateInfo;
			memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			memoryAllocateInfo.pNext = nullptr;
			memoryAllocateInfo.allocationSize = memoryRequirements2.memoryRequirements.size;
			m_memoryManager->FindMemoryTypeFromProperties(memoryRequirements2.memoryRequirements.memoryTypeBits,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &memoryAllocateInfo.memoryTypeIndex);
			result = vkAllocateMemory(Device::Get().m_device, &memoryAllocateInfo, nullptr, &blas.m_accelerationStructureMemory);
			if (result != VK_SUCCESS)
			{
				Logger::Log("Could not allocate memory for bottom level acceleration structure.");
				return false;
			}

			VkBindAccelerationStructureMemoryInfoNV accelerationStructureMemoryInfo;
			accelerationStructureMemoryInfo.sType = VK_STRUCTURE_TYPE_BIND_ACCELERATION_STRUCTURE_MEMORY_INFO_NV;
			accelerationStructureMemoryInfo.pNext = nullptr;
			accelerationStructureMemoryInfo.accelerationStructure = blas.m_accelerationStructure;
			accelerationStructureMemoryInfo.memory = blas.m_accelerationStructureMemory;
			accelerationStructureMemoryInfo.memoryOffset = 0;
			accelerationStructureMemoryInfo.deviceIndexCount = 0;
			accelerationStructureMemoryInfo.pDeviceIndices = nullptr;
			result = vkBindAccelerationStructureMemoryNV(Device::Get().m_device, 1, &accelerationStructureMemoryInfo);
			if (result != VK_SUCCESS)
			{
				Logger::Log("Could not bind memory for bottom level acceleration structure.");
				return false;
			}

			result = vkGetAccelerationStructureHandleNV(Device::Get().m_device, blas.m_accelerationStructure, sizeof(uint64_t), &blas.m_handle);
			if (result != VK_SUCCESS)
			{
				Logger::Log("Could not get handle for bottom level acceleration structure.");
				return false;
			}

			if (memoryRequirements2.memoryRequirements.size > maxScratchSize)
				maxScratchSize = memoryRequirements2.memoryRequirements.size;
		}

		//simple scratch buffer approach, can be expanded to build several acceleration structures simultaneously
		//TODO: look at compaction to reduce memory footprint
		VkBuffer scratchBuffer;
		VkDeviceMemory scratchBufferMemory;
		if (!m_memoryManager->CreateBuffer(maxScratchSize, VK_BUFFER_USAGE_RAY_TRACING_BIT_NV, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, scratchBuffer, scratchBufferMemory))
		{
			Logger::Log("Could not create scratch buffer for bottom level acceleration structure.");
			return false;
		}

		VkCommandBuffer buildBLASCmdBuffer;
		if(!m_memoryManager->CreateSingleUseCommand(buildBLASCmdBuffer))
		{
			Logger::Log("Could not create single use cmd buffer for building BLAS.");
			return false;
		}
		for (auto& blas : m_blasVector)
		{
			vkCmdBuildAccelerationStructureNV(buildBLASCmdBuffer, &blas.m_accelerationStructureInfo, VK_NULL_HANDLE, 0, VK_FALSE,
				blas.m_accelerationStructure, VK_NULL_HANDLE, scratchBuffer, 0);

			VkMemoryBarrier memoryBarrier;
			memoryBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
			memoryBarrier.pNext = nullptr;
			memoryBarrier.srcAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_NV | VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_NV;
			memoryBarrier.dstAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_NV | VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_NV;
			vkCmdPipelineBarrier(buildBLASCmdBuffer, VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_NV, VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_NV,
				0, 1, &memoryBarrier, 0, 0, 0, 0);
		}
		if(!m_memoryManager->EndSingleUseCommand(buildBLASCmdBuffer))
		{
			Logger::Log("Could not end single use cmd buffer for building BLAS.");
			return false;
		}

		vkDestroyBuffer(Device::Get().m_device, scratchBuffer, nullptr);
		vkFreeMemory(Device::Get().m_device, scratchBufferMemory, nullptr);

		return true;
	}

	bool PipelineRaytracing::CreateTLAS()
	{
		m_instanceTranforms.emplace_back(mat3x4(1.f, 0.f, 0.f, 2.f, 0.f, 1.f, 0.f, 2.f, 0.f, 0.f, 1.f, 0.f));
		m_instanceTranforms.emplace_back(mat3x4(1.f, 0.f, 0.f, -2.f, 0.f, 1.f, 0.f, -2.f, 0.f, 0.f, 1.f, 0.f));
		m_instanceTranforms.emplace_back(mat3x4(1.f, 0.f, 0.f, 2.f, 0.f, 1.f, 0.f, -2.f, 0.f, 0.f, 1.f, 0.f));
		m_instanceTranforms.emplace_back(mat3x4(1.f, 0.f, 0.f, -2.f, 0.f, 1.f, 0.f, 2.f, 0.f, 0.f, 1.f, 0.f));

		for (int i = 0; i < m_instanceTranforms.size(); i++)
		{
			BLASInstance blasInstance;
			blasInstance.m_transform = m_instanceTranforms[i];
			blasInstance.m_instanceId = i;
			blasInstance.m_mask = 0xff;
			blasInstance.m_instanceOffset = 0;
			blasInstance.m_flags = VK_GEOMETRY_INSTANCE_TRIANGLE_CULL_DISABLE_BIT_NV;
			blasInstance.m_accelerationStructureHandle = m_blasVector[i].m_handle;

			m_blasInstances.emplace_back(blasInstance);
		}
		VkBuffer instanceBuffer;
		VkDeviceMemory instanceBufferMemory;
		if (!m_memoryManager->CreateOptimalBuffer(instanceBuffer, instanceBufferMemory, m_blasInstances.data(), m_blasInstances.size(), VK_BUFFER_USAGE_RAY_TRACING_BIT_NV))
		{
			Logger::Log("Could not create buffer for blas instance data.");
			return false;
		}

		m_tlas.m_accelerationStructureInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_INFO_NV;
		m_tlas.m_accelerationStructureInfo.pNext = nullptr;
		m_tlas.m_accelerationStructureInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_NV;
		m_tlas.m_accelerationStructureInfo.flags = 0;
		m_tlas.m_accelerationStructureInfo.geometryCount = 0;
		m_tlas.m_accelerationStructureInfo.pGeometries = nullptr;
		m_tlas.m_accelerationStructureInfo.instanceCount = m_blasInstances.size();
		
		VkAccelerationStructureCreateInfoNV accelerationStructureCreateInfo;
		accelerationStructureCreateInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_NV;
		accelerationStructureCreateInfo.pNext = nullptr;
		accelerationStructureCreateInfo.compactedSize = 0;
		accelerationStructureCreateInfo.info = m_tlas.m_accelerationStructureInfo;

		VkResult result = vkCreateAccelerationStructureNV(Device::Get().m_device, &accelerationStructureCreateInfo, nullptr, &m_tlas.m_accelerationStructure);
		if (result != VK_SUCCESS)
		{
			Logger::Log("Could not create bottom level acceleration structure.");
			return false;
		}

		VkAccelerationStructureMemoryRequirementsInfoNV memoryRequirementsInfo;
		memoryRequirementsInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_INFO_NV;
		memoryRequirementsInfo.pNext = nullptr;
		memoryRequirementsInfo.type = VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_OBJECT_NV;
		memoryRequirementsInfo.accelerationStructure = m_tlas.m_accelerationStructure;

		VkMemoryRequirements2 memoryRequirements2 = {};
		vkGetAccelerationStructureMemoryRequirementsNV(Device::Get().m_device, &memoryRequirementsInfo, &memoryRequirements2);

		VkMemoryAllocateInfo memoryAllocateInfo;
		memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		memoryAllocateInfo.pNext = nullptr;
		memoryAllocateInfo.allocationSize = memoryRequirements2.memoryRequirements.size;
		m_memoryManager->FindMemoryTypeFromProperties(memoryRequirements2.memoryRequirements.memoryTypeBits,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &memoryAllocateInfo.memoryTypeIndex);
		result = vkAllocateMemory(Device::Get().m_device, &memoryAllocateInfo, nullptr, &m_tlas.m_accelerationStructureMemory);
		if (result != VK_SUCCESS)
		{
			Logger::Log("Could not allocate memory for bottom level acceleration structure.");
			return false;
		}

		VkBindAccelerationStructureMemoryInfoNV accelerationStructureMemoryInfo;
		accelerationStructureMemoryInfo.sType = VK_STRUCTURE_TYPE_BIND_ACCELERATION_STRUCTURE_MEMORY_INFO_NV;
		accelerationStructureMemoryInfo.pNext = nullptr;
		accelerationStructureMemoryInfo.accelerationStructure = m_tlas.m_accelerationStructure;
		accelerationStructureMemoryInfo.memory = m_tlas.m_accelerationStructureMemory;
		accelerationStructureMemoryInfo.memoryOffset = 0;
		accelerationStructureMemoryInfo.deviceIndexCount = 0;
		accelerationStructureMemoryInfo.pDeviceIndices = nullptr;
		result = vkBindAccelerationStructureMemoryNV(Device::Get().m_device, 1, &accelerationStructureMemoryInfo);
		if (result != VK_SUCCESS)
		{
			Logger::Log("Could not bind memory for bottom level acceleration structure.");
			return false;
		}

		result = vkGetAccelerationStructureHandleNV(Device::Get().m_device, m_tlas.m_accelerationStructure, sizeof(uint64_t), &m_tlas.m_handle);
		if (result != VK_SUCCESS)
		{
			Logger::Log("Could not get handle for bottom level acceleration structure.");
			return false;
		}

		//simple scratch buffer approach, can be expanded to build several acceleration structures simultaneously
		//TODO: look at compaction to reduce memory footprint
		VkBuffer scratchBuffer;
		VkDeviceMemory scratchBufferMemory;
		if (!m_memoryManager->CreateBuffer(memoryRequirements2.memoryRequirements.size, VK_BUFFER_USAGE_RAY_TRACING_BIT_NV, 
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, scratchBuffer, scratchBufferMemory))
		{
			Logger::Log("Could not create scratch buffer for bottom level acceleration structure.");
			return false;
		}

		VkCommandBuffer buildTLASCmdBuffer;
		if (!m_memoryManager->CreateSingleUseCommand(buildTLASCmdBuffer))
		{
			Logger::Log("Could not create single use cmd buffer for building BLAS.");
			return false;
		}
		
		vkCmdBuildAccelerationStructureNV(buildTLASCmdBuffer, &m_tlas.m_accelerationStructureInfo, instanceBuffer, 0, VK_FALSE,
			m_tlas.m_accelerationStructure, VK_NULL_HANDLE, scratchBuffer, 0);

		VkMemoryBarrier memoryBarrier;
		memoryBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
		memoryBarrier.pNext = nullptr;
		memoryBarrier.srcAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_NV | VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_NV;
		memoryBarrier.dstAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_NV | VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_NV;
		vkCmdPipelineBarrier(buildTLASCmdBuffer, VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_NV, VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_NV,
			0, 1, &memoryBarrier, 0, 0, 0, 0);
		
		if (!m_memoryManager->EndSingleUseCommand(buildTLASCmdBuffer))
		{
			Logger::Log("Could not end single use cmd buffer for building BLAS.");
			return false;
		}

		vkDestroyBuffer(Device::Get().m_device, scratchBuffer, nullptr);
		vkFreeMemory(Device::Get().m_device, scratchBufferMemory, nullptr);
		//vkDestroyBuffer(Device::Get().m_device, instanceBuffer, nullptr);
		//vkFreeMemory(Device::Get().m_device, instanceBufferMemory, nullptr);

		return true;
	}

	bool PipelineRaytracing::CreateStorageImage()
	{
		if (!m_memoryManager->CreateImage(m_storageImage, m_storageImageMemory, m_extent, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_STORAGE_BIT))
		{
			Logger::Log("Could not create storage image for raytracing.");
			return false;
		}

		if (!m_memoryManager->CreateImageView(m_storageImageView, m_storageImage))
		{
			Logger::Log("Could not create storage image view for raytracing.");
			return false;
		}

		VkCommandBuffer layoutTransitionCommandBuffer;
		if (!m_memoryManager->CreateSingleUseCommand(layoutTransitionCommandBuffer))
		{
			Logger::Log("Could not create single use command buffer for transition of image layout.");
			return false;
		}
		if (!m_memoryManager->TransitionImageLayout(layoutTransitionCommandBuffer, m_storageImage, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL ))
		{
			Logger::Log("Could not transition storage image layout for raytracing.");
			return false;
		}
		if (!m_memoryManager->EndSingleUseCommand(layoutTransitionCommandBuffer))
		{
			Logger::Log("Could not end command buffer for image layout transition.");
			return false;
		}

		return true;
	}

	bool PipelineRaytracing::CreateShaderBindingTable()
	{
		uint32_t shaderBindingTableSize = m_raytracingProperties->shaderGroupHandleSize * m_rtShaderGroups.size();
		if(!m_memoryManager->CreateBuffer(shaderBindingTableSize, VK_BUFFER_USAGE_RAY_TRACING_BIT_NV, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
			m_shaderBindingTable, m_shaderBindingTableMemory))
		{
			Logger::Log("Could not create shader binding table buffer.");
			return false;
		}

		void* data;
		vkMapMemory(Device::Get().m_device, m_shaderBindingTableMemory, 0, VK_WHOLE_SIZE, 0, &data);

		std::vector<uint8_t> shaderHandles(shaderBindingTableSize);
		VkResult result = vkGetRayTracingShaderGroupHandlesNV(Device::Get().m_device, m_pipeline, 0, m_rtShaderGroups.size(), 
			shaderBindingTableSize, shaderHandles.data());
		if(result != VK_SUCCESS)
		{
			Logger::Log("Could not get raytracing shader group handles.");
			return false;
		}

		memcpy(data, shaderHandles.data(), shaderBindingTableSize);

		vkUnmapMemory(Device::Get().m_device, m_shaderBindingTableMemory);

		return true;
	}

	void PipelineRaytracing::DefineVertices()
	{
		VkVertexInputBindingDescription vertexInputBinding;
		vertexInputBinding.binding = 0;
		vertexInputBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		vertexInputBinding.stride = sizeof(Vertex);
		m_vertexInputBindings.emplace_back(vertexInputBinding);

		VkVertexInputAttributeDescription vertexAttributePosition, vertexAttributeColor, vertexAttributeUV;
		vertexAttributePosition.binding = 0;
		vertexAttributePosition.location = 0;
		vertexAttributePosition.format = VK_FORMAT_R32G32B32_SFLOAT;
		vertexAttributePosition.offset = 0;
		m_vertexInputAttributes.emplace_back(vertexAttributePosition);

		vertexAttributeColor.binding = 0;
		vertexAttributeColor.location = 1;
		vertexAttributeColor.format = VK_FORMAT_R32G32B32_SFLOAT;
		vertexAttributeColor.offset = sizeof(float) * 3;
		m_vertexInputAttributes.emplace_back(vertexAttributeColor);

		vertexAttributeUV.binding = 0;
		vertexAttributeUV.location = 2;
		vertexAttributeUV.format = VK_FORMAT_R32G32_SFLOAT;
		vertexAttributeUV.offset = sizeof(float) * 6;
		m_vertexInputAttributes.emplace_back(vertexAttributeUV);
	}

	bool PipelineRaytracing::CreateShaderModules()
	{
		auto raygenShaderCode = readFile("shaders/rgen.spv");
		auto missShaderCode = readFile("shaders/rmiss.spv");
		auto hitShaderCode = readFile("shaders/rchit.spv");

		VkPipelineShaderStageCreateInfo raygenShader, missShader, hitShader;

		CreateShaderModule(raygenShaderCode, raygenShader.module);
		CreateShaderModule(missShaderCode, missShader.module);
		CreateShaderModule(hitShaderCode, hitShader.module);

		raygenShader.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		raygenShader.pNext = nullptr;
		raygenShader.flags = 0;
		raygenShader.stage = VK_SHADER_STAGE_RAYGEN_BIT_NV;
		raygenShader.pName = "main";
		raygenShader.pSpecializationInfo = nullptr;

		missShader.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		missShader.pNext = nullptr;
		missShader.flags = 0;
		missShader.stage = VK_SHADER_STAGE_MISS_BIT_NV;
		missShader.pName = "main";
		missShader.pSpecializationInfo = nullptr;

		hitShader.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		hitShader.pNext = nullptr;
		hitShader.flags = 0;
		hitShader.stage = VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV;
		hitShader.pName = "main";
		hitShader.pSpecializationInfo = nullptr;

		VkRayTracingShaderGroupCreateInfoNV raygenGroupInfo, missGroupInfo, hitGroupInfo = {};

		raygenGroupInfo.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_NV;
		raygenGroupInfo.pNext = nullptr;
		raygenGroupInfo.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_NV;
		raygenGroupInfo.generalShader = 0;
		raygenGroupInfo.closestHitShader = VK_SHADER_UNUSED_NV;
		raygenGroupInfo.anyHitShader = VK_SHADER_UNUSED_NV;
		raygenGroupInfo.intersectionShader = VK_SHADER_UNUSED_NV;

		missGroupInfo.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_NV;
		missGroupInfo.pNext = nullptr;
		missGroupInfo.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_NV;
		missGroupInfo.generalShader = 1;
		missGroupInfo.closestHitShader = VK_SHADER_UNUSED_NV;
		missGroupInfo.anyHitShader = VK_SHADER_UNUSED_NV;
		missGroupInfo.intersectionShader = VK_SHADER_UNUSED_NV;

		hitGroupInfo.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_NV;
		hitGroupInfo.pNext = nullptr;
		hitGroupInfo.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_NV;
		hitGroupInfo.generalShader = VK_SHADER_UNUSED_NV;
		hitGroupInfo.closestHitShader = 2;
		hitGroupInfo.anyHitShader = VK_SHADER_UNUSED_NV;
		hitGroupInfo.intersectionShader = VK_SHADER_UNUSED_NV;

		m_shaderStagesV.emplace_back(raygenShader);
		m_shaderStagesV.emplace_back(missShader);
		m_shaderStagesV.emplace_back(hitShader);

		m_rtShaderGroups.emplace_back(raygenGroupInfo);
		m_rtShaderGroups.emplace_back(missGroupInfo);
		m_rtShaderGroups.emplace_back(hitGroupInfo);

		return true;
	}

	bool PipelineRaytracing::CreateGraphicsPipeline()
	{
		VkRayTracingPipelineCreateInfoNV raytracePipleineInfo;
		raytracePipleineInfo.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_NV;
		raytracePipleineInfo.pNext = nullptr;
		raytracePipleineInfo.flags = 0;
		raytracePipleineInfo.stageCount = m_shaderStagesV.size();
		raytracePipleineInfo.pStages = m_shaderStagesV.data();
		raytracePipleineInfo.groupCount = m_rtShaderGroups.size();
		raytracePipleineInfo.pGroups = m_rtShaderGroups.data();
		raytracePipleineInfo.maxRecursionDepth = 1;
		raytracePipleineInfo.layout = m_pipelineLayout;
		raytracePipleineInfo.basePipelineIndex = 0;
		VkResult result = vkCreateRayTracingPipelinesNV(Device::Get().m_device, VK_NULL_HANDLE, 1, &raytracePipleineInfo, nullptr, &m_pipeline);
		if (result != VK_SUCCESS)
		{
			Logger::Log("Could not create raytracing pipeline.");
			return false;
		}

		return true;
	}

	bool PipelineRaytracing::Draw(VkCommandBuffer& commandBuffer)
	{
		m_rtPushConstants.clearColor = { 1.f, 0.f, 0.f, 1.f };
		m_rtPushConstants.lightPosition = { 1.f, 10.f, 0.f };
		m_rtPushConstants.lightIntensity = 1.f;
		m_rtPushConstants.lightType = 0;

		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_NV, m_pipeline);
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_NV, m_pipelineLayout, 0, m_descriptorSets.size(), m_descriptorSets.data(),
			0, nullptr);
		vkCmdPushConstants(commandBuffer, m_pipelineLayout, VK_SHADER_STAGE_RAYGEN_BIT_NV | VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV | VK_SHADER_STAGE_MISS_BIT_NV,
			0, sizeof(RtPushConstant), &m_rtPushConstants);

		VkDeviceSize bindingStride = m_raytracingProperties->shaderGroupHandleSize;
		VkDeviceSize bindingOffsetRayGenShader = 0;
		VkDeviceSize bindingOffsetMissShader = bindingStride;
		VkDeviceSize bindingOffsetHitShader = bindingStride*2;

		vkCmdTraceRaysNV(commandBuffer, 
			m_shaderBindingTable, bindingOffsetRayGenShader,
			m_shaderBindingTable, bindingOffsetMissShader, bindingStride,
			m_shaderBindingTable, bindingOffsetHitShader, bindingStride,
			VK_NULL_HANDLE, 0, 0,
			m_extent.width, m_extent.height, 1);

		return true;
	}

	bool PipelineRaytracing::CreatePipelineLayout()
	{
		std::vector<VkDescriptorSetLayoutBinding> layoutBindings;
		unsigned int layoutBindingIndex = 0;

		VkDescriptorSetLayoutBinding accelerationStructureLayoutBinding = {
			layoutBindingIndex++,
			VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV,
			1,
			VK_SHADER_STAGE_RAYGEN_BIT_NV,
			nullptr
		};
		layoutBindings.emplace_back(accelerationStructureLayoutBinding);

		VkDescriptorSetLayoutBinding writeImageLayoutBinding = {
			layoutBindingIndex++,
			VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
			1,
			VK_SHADER_STAGE_RAYGEN_BIT_NV,
			nullptr
		};
		layoutBindings.emplace_back(writeImageLayoutBinding);

		/*
		VkDescriptorSetLayoutBinding samplerLayoutBinding = {
			layoutBindingIndex++,
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			1,
			VK_SHADER_STAGE_RAYGEN_BIT_NV,
			nullptr
		};
		layoutBindings.emplace_back(samplerLayoutBinding);
		*/

		VkDescriptorSetLayoutCreateInfo descriptorLayoutInfo = {
			VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
			nullptr,
			0,
			layoutBindings.size(),
			layoutBindings.data()
		};
		m_descriptorSetLayouts.resize(1);
		VkResult result = vkCreateDescriptorSetLayout(Device::Get().m_device, &descriptorLayoutInfo, nullptr, m_descriptorSetLayouts.data());
		if (result != VK_SUCCESS)
		{
			Logger::Log("Could not create uniform buffer descriptor set layout.");
			return false;
		}

		std::vector<VkPushConstantRange> pushConstantRanges;
		VkPushConstantRange pushConstantRangeTransforms = {};
		pushConstantRangeTransforms.stageFlags = VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV | VK_SHADER_STAGE_MISS_BIT_NV | VK_SHADER_STAGE_RAYGEN_BIT_NV;
		pushConstantRangeTransforms.offset = 0;
		pushConstantRangeTransforms.size = sizeof(RtPushConstant);
		pushConstantRanges.emplace_back(pushConstantRangeTransforms);

		VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {
			VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
			nullptr,
			0,
			m_descriptorSetLayouts.size(),
			m_descriptorSetLayouts.data(),
			pushConstantRanges.size(),
			pushConstantRanges.data()
		};
		result = vkCreatePipelineLayout(Device::Get().m_device, &pipelineLayoutCreateInfo, nullptr, &m_pipelineLayout);
		if (result != VK_SUCCESS)
		{
			Logger::Log("Could not create pipeline layout.");
			return false;
		}

		return true;
	}

	bool PipelineRaytracing::CreateDescriptorPool()
	{
		std::vector<VkDescriptorPoolSize> descriptorPoolSizes;
		VkDescriptorPoolSize poolSizeViewProjection = {};
		poolSizeViewProjection.type = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV;
		poolSizeViewProjection.descriptorCount = 1;
		descriptorPoolSizes.emplace_back(poolSizeViewProjection);

		VkDescriptorPoolSize poolSizeDynamicTransform = {};
		poolSizeDynamicTransform.type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		poolSizeDynamicTransform.descriptorCount = 1;
		descriptorPoolSizes.emplace_back(poolSizeDynamicTransform);

		/*
		VkDescriptorPoolSize poolSizeTextureSampler = {};
		poolSizeTextureSampler.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSizeTextureSampler.descriptorCount = 1;
		descriptorPoolSizes.emplace_back(poolSizeTextureSampler);
		*/

		VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = {
			VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
			nullptr,
			0,
			1, //TODO: max sets, make non constant, take number from CreatePipelineLayout
			descriptorPoolSizes.size(),
			descriptorPoolSizes.data()
		};
		VkResult result = vkCreateDescriptorPool(Device::Get().m_device, &descriptorPoolCreateInfo, nullptr, &m_descriptorPool);
		if (result != VK_SUCCESS)
		{
			Logger::Log("Could not create descriptor pool.");
			return false;
		}

		return true;
	}

	bool PipelineRaytracing::CreateDescriptorSet()
	{
		VkDescriptorSetAllocateInfo allocInfo;
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.pNext = nullptr;
		allocInfo.descriptorPool = m_descriptorPool;
		allocInfo.descriptorSetCount = m_descriptorSetLayouts.size();
		allocInfo.pSetLayouts = m_descriptorSetLayouts.data();
		m_descriptorSets.resize(m_descriptorSetLayouts.size());
		VkResult result = vkAllocateDescriptorSets(Device::Get().m_device, &allocInfo, m_descriptorSets.data());
		if (result != VK_SUCCESS)
		{
			Logger::Log("Could not allocate descriptor set.");
			return false;
		}

		std::vector<VkWriteDescriptorSet> writes;
		uint32_t dstBinding = 0;

		VkWriteDescriptorSetAccelerationStructureNV descriptorAccelerationStructureInfo;
		descriptorAccelerationStructureInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_NV;
		descriptorAccelerationStructureInfo.pNext = nullptr;
		descriptorAccelerationStructureInfo.accelerationStructureCount = 1;
		descriptorAccelerationStructureInfo.pAccelerationStructures = &m_tlas.m_accelerationStructure;

		VkWriteDescriptorSet accelerationStructureDescriptorSet;
		accelerationStructureDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		accelerationStructureDescriptorSet.pNext = &descriptorAccelerationStructureInfo;
		accelerationStructureDescriptorSet.dstSet = m_descriptorSets[0];
		accelerationStructureDescriptorSet.descriptorCount = 1;
		accelerationStructureDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV;
		accelerationStructureDescriptorSet.pBufferInfo = nullptr;
		accelerationStructureDescriptorSet.dstArrayElement = 0;
		accelerationStructureDescriptorSet.dstBinding = dstBinding++;
		writes.emplace_back(accelerationStructureDescriptorSet);


		VkDescriptorImageInfo storageImageDescriptor;
		storageImageDescriptor.imageView = m_storageImageView;
		storageImageDescriptor.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
		storageImageDescriptor.sampler = VK_NULL_HANDLE;

		VkWriteDescriptorSet resultWriteImageDescriptorSet;
		resultWriteImageDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		resultWriteImageDescriptorSet.pNext = nullptr;
		resultWriteImageDescriptorSet.dstSet = m_descriptorSets[0];
		resultWriteImageDescriptorSet.descriptorCount = 1;
		resultWriteImageDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		resultWriteImageDescriptorSet.pBufferInfo = nullptr;
		resultWriteImageDescriptorSet.pImageInfo = &storageImageDescriptor;
		resultWriteImageDescriptorSet.dstArrayElement = 0;
		resultWriteImageDescriptorSet.dstBinding = dstBinding++;
		writes.emplace_back(resultWriteImageDescriptorSet);

		/*
		VkWriteDescriptorSet uniformWriteBufferDescriptorSet;
		uniformWriteBufferDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		uniformWriteBufferDescriptorSet.pNext = nullptr;
		uniformWriteBufferDescriptorSet.dstSet = m_descriptorSets[0];
		uniformWriteBufferDescriptorSet.descriptorCount = 1;
		uniformWriteBufferDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uniformWriteBufferDescriptorSet.pBufferInfo = ;
		uniformWriteBufferDescriptorSet.pImageInfo = ;
		uniformWriteBufferDescriptorSet.dstArrayElement = 0;
		uniformWriteBufferDescriptorSet.dstBinding = dstBinding++;
		writes.emplace_back(uniformWriteBufferDescriptorSet);
		*/

		vkUpdateDescriptorSets(Device::Get().m_device, writes.size(), writes.data(), 0, nullptr);

		return true;
	}
}
