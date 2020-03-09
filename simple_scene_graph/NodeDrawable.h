#pragma once

#include "Node.h"
#include "../Drawable.h"

namespace MelonRenderer
{
	class NodeDrawable :
		public Node
	{
	public:
		mat4 CalculateWorldTransform(const mat4& parentMat);
		bool IsStatic();
		void SetStatic(bool isStatic);
		
		void SetDrawableInstance(DrawableInstance* drawableInstance);

	protected:
		DrawableInstance* m_drawableInstance;
		bool m_isStatic;
	};
}