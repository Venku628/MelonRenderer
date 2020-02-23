#include "NodeDrawable.h"

void MelonRenderer::NodeDrawable::Tick(const mat4& parentMat)
{
	mat4 actualTransformationMat = m_transformationMat * parentMat;

	//m_drawable->Tick(...);

	for (auto child : m_children)
	{
		child->Tick(actualTransformationMat);
	}
}

void MelonRenderer::NodeDrawable::Tick()
{
	for (auto child : m_children)
	{
		child->Tick(m_transformationMat);
	}

	//m_drawable->Tick(...);
}
