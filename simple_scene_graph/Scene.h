#pragma once
#include "NodeDrawable.h"
#include "NodeCamera.h"

namespace MelonRenderer
{
	class Scene
	{
	public:
		void Tick(PipelineData* pipelineData);

		std::vector<Node*> m_rootChildren;
	};
}