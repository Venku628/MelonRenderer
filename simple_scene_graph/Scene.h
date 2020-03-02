#pragma once
#include "NodeDrawable.h"
#include "NodeCamera.h"

namespace MelonRenderer
{
	class Scene
	{
	public:
		void Tick(PipelineData& pipelineData);

		//Rt function
		std::vector<Drawable*> SearchForDynamicDrawables();
		//TODO: GetStaticDrawables Rt function

		std::vector<Node*> m_rootChildren;
	};
}