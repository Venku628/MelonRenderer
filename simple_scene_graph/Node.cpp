#include "Node.h"

mat4 MelonRenderer::Node::CalculateWorldTransform(const mat4& parentMat, uint32_t** handle)
{
	*handle = nullptr;
	return parentMat * m_transformationMat;
}

mat4* MelonRenderer::Node::GetTransformMat()
{
	return &m_transformationMat;
}


