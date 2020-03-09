#include "Node.h"

mat4 MelonRenderer::Node::CalculateWorldTransform(const mat4& parentMat)
{
	return m_transformationMat * parentMat;
}

mat4* MelonRenderer::Node::GetTransformMat()
{
	return &m_transformationMat;
}


