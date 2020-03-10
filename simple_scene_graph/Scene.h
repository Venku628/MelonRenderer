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

		//returns a handle to give to a NodeDrawable
		uint32_t CreateDrawableInstance(uint32_t drawableHandle, bool isStatic);

		//TODO: save seperatley, to allow nodes direct access, without requesting a handle from them
		std::vector<Drawable> m_drawables;
		//simple solution to group objects together for now
		std::vector<bool> m_drawableInstanceIsStatic;
		std::vector<DrawableInstance> m_drawableInstances;
	};
}