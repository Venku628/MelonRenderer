#include "Drawable.h"

namespace MelonRenderer {
	bool Drawable::LoadMeshData()
	{
		


		return true;
	}

	bool Drawable::Init(DeviceMemoryManager& memoryManager)
	{
		//TODO: make drawable attribute
		uint32_t vertexBufferSize = sizeof(cube_vertex_data);
		if (!memoryManager.CreateOptimalBuffer(m_vertexBuffer, m_vertexBufferMemory, cube_vertex_data, vertexBufferSize, 
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT))
		{
			Logger::Log("Could not create vertex buffer.");
			return false;
		}

		//TODO: make drawable attribute
		uint32_t indexBufferSize = sizeof(cube_index_data);
		if (!memoryManager.CreateOptimalBuffer(m_indexBuffer, m_indexBufferMemory, cube_index_data, indexBufferSize, 
			VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT))
		{
			Logger::Log("Could not create index buffer.");
			return false;
		}

		m_vertexCount = sizeof(cube_vertex_data) / sizeof(Vertex);
		m_indexCount = sizeof(cube_index_data) / sizeof(uint32_t);

		return true;
	}

	void Drawable::Tick(PipelineData& pipelineData)
	{
		const VkDeviceSize offsets[1] = { 0 };
		vkCmdBindVertexBuffers(*pipelineData.m_commandBuffer, 0, 1, &m_vertexBuffer, offsets);
		vkCmdBindIndexBuffer(*pipelineData.m_commandBuffer, m_indexBuffer, 0, VK_INDEX_TYPE_UINT32);

		//TODO: do once for every instance
		vkCmdBindDescriptorSets(*pipelineData.m_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, *pipelineData.m_pipelineLayout, 0, 
			pipelineData.m_descriptorSets->size(), pipelineData.m_descriptorSets->data(),
			1, pipelineData.m_transformOffset);

		vkCmdDrawIndexed(*pipelineData.m_commandBuffer, sizeof(cube_index_data) / sizeof(uint32_t), 1, 0, 0, 0);

		*pipelineData.m_transformOffset += pipelineData.m_alignment;
	}
}