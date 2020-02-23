#pragma once

#include "Node.h"
#include "../Drawable.h"

namespace MelonRenderer
{
	class NodeDrawable :
		public Node
	{
	public:
		void Tick(const mat4& parentMat);
		void Tick();

	protected:
		Drawable* m_drawable;
	};
}