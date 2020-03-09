#include "NodeDrawable.h"

mat4 MelonRenderer::NodeDrawable::CalculateWorldTransform(const mat4& parentMat)
{
	auto worldTransformMat = m_transformationMat * parentMat;
	m_drawableInstance->m_transformation = worldTransformMat;
	m_drawableInstance->m_transformationInverseTranspose = glm::inverse(glm::transpose(worldTransformMat));
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

void MelonRenderer::NodeDrawable::SetDrawableInstance(DrawableInstance* drawableInstance)
{
	m_drawableInstance = drawableInstance;
}
