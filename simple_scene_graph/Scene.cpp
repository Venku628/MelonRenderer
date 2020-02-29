#include "Scene.h"

void MelonRenderer::Scene::Tick(PipelineData& pipelineData)
{
	for (auto rootChild : m_rootChildren)
	{
		rootChild->Tick(pipelineData);
	}
}
