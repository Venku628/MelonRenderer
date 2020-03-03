#pragma once

#include <vector>

#include "../Basics.h"
#include "../Drawable.h"

namespace MelonRenderer
{
	class Node
	{
	public:
		virtual void Tick(PipelineData& pipelineData, const mat4& parentMat);
		virtual void Tick(PipelineData& pipelineData);
		virtual void SearchForDynamicDrawables(std::vector<Drawable*>* drawables);

		mat4 m_transformationMat;

	protected:
		std::vector<Node*> m_children;
	};
}