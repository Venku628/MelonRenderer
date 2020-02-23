#pragma once

#include <vector>

#include "../Basics.h"

namespace MelonRenderer
{
	class Node
	{
	public:
		void Tick(const mat4& parentMat);
		void Tick();

		mat4 m_transformationMat;

	protected:
		std::vector<Node*> m_children;
	};
}