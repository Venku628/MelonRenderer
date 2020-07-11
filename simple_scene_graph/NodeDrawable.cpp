#include "NodeDrawable.h"

mat4 MelonRenderer::NodeDrawable::CalculateWorldTransform(const mat4& parentMat, uint32_t** handle)
{
	auto worldTransformMat = parentMat * m_transformationMat;
	*handle = &m_drawableInstance;
	return worldTransformMat;
}

void MelonRenderer::NodeDrawable::SetDrawableInstance(uint32_t drawableInstance)
{
	m_drawableInstance = drawableInstance;
}
