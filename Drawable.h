#pragma once

#include "Basics.h"
#include "cube.h"

namespace MelonRenderer {
	//TODO: eventually replace Vertex with VertexNeu
	struct VertexNeu {
		float posX, posY, posZ;
		float normalX, normalY, normalZ;
		float tangentX, tangentY, tangentZ;
		float bitangentX, bitangentY, bitangentZ;
		float texU, texV;
	};

	class Drawable
	{
	public:
		bool LoadMeshData();

		void Tick(VkCommandBuffer& commandBuffer, VkPipelineLayout& pipelineLayout);

	protected:
		Vertex* m_vertexData;
		VkBuffer m_vertexBuffer;
		VkDeviceMemory m_vertexBufferMemory;

		MeshIndex* m_indexData;
		VkBuffer m_indexBuffer;
		VkDeviceMemory m_indexBufferMemory;

		//DEBUG
		mat4 m_transform = mat4(1, 0, 0, 0,
			0, 1, 0, -2, 
			0, 0, 1, 0, 
			0, 0, 0, 1);

		friend class Renderer;
	};
}