#pragma once

#include "Node.h"
#include "../Drawable.h"

namespace MelonRenderer
{
	class NodeDrawable :
		public Node
	{
	public:
		mat4 CalculateWorldTransform(const mat4& parentMat, uint32_t** handle) override;
		bool IsStatic();
		void SetStatic(bool isStatic);
		
		void SetDrawableInstance(uint32_t drawableInstance);

	protected:
		uint32_t m_drawableInstance;
		bool m_isStatic;
	};
}