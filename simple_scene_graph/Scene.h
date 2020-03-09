#pragma once
#include "NodeDrawable.h"
#include "NodeCamera.h"
#include <stack>
#include <utility>

namespace MelonRenderer
{
	class Scene
	{
	public:
		std::vector<Node*> m_rootChildren;

		void UpdateInstanceTransforms();

		uint32_t CreateDrawableInstance(uint32_t drawableHandle, bool isStatic);

		std::vector<Drawable> m_drawables;
		//simple solution to group objects together for now
		std::vector<bool> m_drawableInstanceIsStatic;
		std::vector<DrawableInstance> m_drawableInstances;
	};
}