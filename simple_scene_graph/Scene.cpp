#include "Scene.h"

namespace MelonRenderer
{
	void Scene::Tick(PipelineData& pipelineData)
	{
		for (auto rootChild : m_rootChildren)
		{
			rootChild->Tick(pipelineData);
		}
	}

	std::vector<Drawable*> Scene::SearchForDynamicDrawables()
	{
		std::vector<Drawable*> dynamicDrawables;

		for (auto rootChild : m_rootChildren)
		{
			rootChild->SearchForDynamicDrawables(dynamicDrawables);
		}

		return dynamicDrawables;
	}
}