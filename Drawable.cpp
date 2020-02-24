#include "Drawable.h"

namespace MelonRenderer {
	bool Drawable::LoadMeshData()
	{
		//TODO: do this 


		return true;
	}

	bool Drawable::Init(DeviceMemoryManager& memoryManager)
	{
		//TODO: make drawable attribute
		uint32_t vertexBufferSize = sizeof(cube_vertex_data);
		if (!memoryManager.CreateOptimalBuffer(m_vertexBuffer, m_vertexBufferMemory, cube_vertex_data, vertexBufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT))
		{
			Logger::Log("Could not create vertex buffer.");
			return false;
		}

		//TODO: make drawable attribute
		uint32_t indexBufferSize = sizeof(cube_index_data);
		if (!memoryManager.CreateOptimalBuffer(m_indexBuffer, m_indexBufferMemory, cube_index_data, indexBufferSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT))
		{
			Logger::Log("Could not create index buffer.");
			return false;
		}

		return true;
	}

	void Drawable::Tick(VkCommandBuffer& commandBuffer, VkPipelineLayout& pipelineLayout, ObjectData& objectData)
	{
		const VkDeviceSize offsets[1] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, &m_vertexBuffer, offsets);
		vkCmdBindIndexBuffer(commandBuffer, m_indexBuffer, 0, VK_INDEX_TYPE_UINT32);

		vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(mat4), &objectData.transformMatrix);
		vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, sizeof(mat4), sizeof(objectData.materialIndices), &objectData.materialIndices);
		vkCmdDrawIndexed(commandBuffer, sizeof(cube_index_data) / sizeof(uint32_t), 1, 0, 0, 0);
	}
}