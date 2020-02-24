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

	struct ObjectData {
		mat4 transformMatrix;
		uint32_t materialIndices[1];
	};

	class Drawable
	{
	public:
		bool LoadMeshData();

		bool Init(DeviceMemoryManager& memoryManager);
		void Tick(VkCommandBuffer& commandBuffer, VkPipelineLayout& pipelineLayout, ObjectData& objectData);

	protected:
		Vertex* m_vertexData;
		VkBuffer m_vertexBuffer;
		VkDeviceMemory m_vertexBufferMemory;

		MeshIndex* m_indexData;
		VkBuffer m_indexBuffer;
		VkDeviceMemory m_indexBufferMemory;

		friend class Pipeline;
		friend class PipelineRasterization;
	};
}