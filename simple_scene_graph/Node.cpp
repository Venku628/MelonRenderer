#include "Node.h"

void MelonRenderer::Node::Tick(PipelineData* pipelineData, const mat4& parentMat)
{
	if (m_children.size() == 0)
		return;

	mat4 actualTransformationMat = m_transformationMat * parentMat;
	for (auto child : m_children)
	{
		child->Tick(pipelineData, actualTransformationMat);
	}
}

void MelonRenderer::Node::Tick(PipelineData* pipelineData)
{
	for (auto child : m_children)
	{
		child->Tick(pipelineData, m_transformationMat);
	}
}
