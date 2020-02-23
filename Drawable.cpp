#include "Drawable.h"

namespace MelonRenderer {
	bool Drawable::LoadMeshData()
	{
		//TODO: do this 


		return true;
	}

	void Drawable::Tick(VkCommandBuffer& commandBuffer, VkPipelineLayout& pipelineLayout)
	{
		const VkDeviceSize offsets[1] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, &m_vertexBuffer, offsets);
		vkCmdBindIndexBuffer(commandBuffer, m_indexBuffer, 0, VK_INDEX_TYPE_UINT32);

		vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(mat4), &m_objectData.transformMatrix);
		vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, sizeof(mat4), sizeof(m_objectData.materialIndices), &m_objectData.materialIndices);
		vkCmdDrawIndexed(commandBuffer, sizeof(cube_index_data) / sizeof(uint32_t), 1, 0, 0, 0);
	}
}