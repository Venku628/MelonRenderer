#pragma once
#include "NodeDrawable.h"

namespace MelonRenderer
{
	class Scene
	{
	public:
		void Tick();

		std::vector<Node*> m_rootChildren;
	};

}