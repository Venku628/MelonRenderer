#include "NodeDrawable.h"

mat4 MelonRenderer::NodeDrawable::CalculateWorldTransform(const mat4& parentMat, uint32_t** handle)
{
	auto worldTransformMat = m_transformationMat * parentMat;
	*handle = &m_drawableInstance;
	return worldTransformMat;
}

bool MelonRenderer::NodeDrawable::IsStatic()
{
	return m_isStatic;
}

void MelonRenderer::NodeDrawable::SetStatic(bool isStatic)
{
	m_isStatic = isStatic;
}

void MelonRenderer::NodeDrawable::SetDrawableInstance(uint32_t drawableInstance)
{
	m_drawableInstance = drawableInstance;
}
