#pragma once
#include "NodeDrawable.h"
#include "NodeCamera.h"

namespace MelonRenderer
{
	class Scene
	{
	public:
		void Tick(VkCommandBuffer* m_commandBuffer,	VkPipelineLayout* m_pipelineLayout);

		std::vector<Node*> m_rootChildren;
	};
}