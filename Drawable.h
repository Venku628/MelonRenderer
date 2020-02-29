#pragma once

#include "Basics.h"
#include "cube.h"
#include "DeviceMemoryManager.h"

namespace MelonRenderer {
	//TODO: eventually replace Vertex with VertexNeu
	struct VertexNeu {
		float posX, posY, posZ;
		float normalX, normalY, normalZ;
		float tangentX, tangentY, tangentZ;
		float bitangentX, bitangentY, bitangentZ;
		float texU, texV;
	};

	typedef uint32_t MeshIndex;

	struct PipelineData
	{
		VkCommandBuffer* m_commandBuffer;
		VkPipelineLayout* m_pipelineLayout;
		std::vector<VkDescriptorSet>* m_descriptorSets;
		uint32_t* m_transformOffset;
		uint32_t m_alignment = 64;
	};

	class Drawable
	{
	public:
		bool LoadMeshData();

		bool Init(DeviceMemoryManager& memoryManager);
		void Tick(PipelineData& pipelineData);

	protected:
		Vertex* m_vertexData;
		VkBuffer m_vertexBuffer;
		VkDeviceMemory m_vertexBufferMemory;
		uint32_t m_vertexCount;

		MeshIndex* m_indexData;
		VkBuffer m_indexBuffer;
		VkDeviceMemory m_indexBufferMemory;
		uint32_t m_indexCount;

		friend class Pipeline;
		friend class PipelineRasterization;
		friend class PipelineRaytracing;
	};
}