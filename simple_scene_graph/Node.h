#pragma once

#include <vector>

#include "../Basics.h"

namespace MelonRenderer
{
	struct PipelineData
	{
		VkCommandBuffer* m_commandBuffer;
		VkPipelineLayout* m_pipelineLayout;
	};

	class Node
	{
	public:
		virtual void Tick(PipelineData* pipelineData, const mat4& parentMat);
		virtual void Tick(PipelineData* pipelineData);

		mat4 m_transformationMat;

	protected:
		std::vector<Node*> m_children;
	};
}