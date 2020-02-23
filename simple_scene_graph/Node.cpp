#include "Node.h"

void MelonRenderer::Node::Tick(const mat4& parentMat)
{
	if (m_children.size() == 0)
		return;

	mat4 actualTransformationMat = m_transformationMat * parentMat;
	for (auto child : m_children)
	{
		child->Tick(actualTransformationMat);
	}
}

void MelonRenderer::Node::Tick()
{
	for (auto child : m_children)
	{
		child->Tick(m_transformationMat);
	}
}
