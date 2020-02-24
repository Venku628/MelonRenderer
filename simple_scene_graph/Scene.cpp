#include "Scene.h"

void MelonRenderer::Scene::Tick(VkCommandBuffer* commandBuffer, VkPipelineLayout* pipelineLayout)
{
	PipelineData pipelineData =
	{
		commandBuffer,
		pipelineLayout
	};

	for (auto rootChild : m_rootChildren)
	{
		rootChild->Tick(&pipelineData);
	}
}
