#include "Scene.h"

void MelonRenderer::Scene::Tick()
{
	for (auto rootChild : m_rootChildren)
	{
		rootChild->Tick();
	}
}
