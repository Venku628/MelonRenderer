#pragma once

#include <vector>

#include "../Basics.h"
#include "../Drawable.h"

namespace MelonRenderer
{
	class Node
	{
	public:
		virtual mat4 CalculateWorldTransform(const mat4& parentMat, uint32_t** handle);
		mat4* GetTransformMat();

	protected:
		mat4 m_transformationMat;
		std::vector<Node*> m_children;

		friend class Scene;
	};
}