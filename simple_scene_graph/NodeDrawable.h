#pragma once

#include "Node.h"
#include "../Drawable.h"

namespace MelonRenderer
{
	class NodeDrawable :
		public Node
	{
	public:
		void Tick(PipelineData& pipelineData, const mat4& parentMat) override;
		void Tick(PipelineData& pipelineData) override;
		void SearchForDynamicDrawables(std::vector<Drawable*> drawables) override;

		void SetDrawable(Drawable* drawable);
		void SetMaterialIndices(uint32_t materialIndice);

	protected:
		Drawable* m_drawable;
		mat3x4 m_transform;
	};
}