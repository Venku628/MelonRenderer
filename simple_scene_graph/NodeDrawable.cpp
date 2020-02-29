#include "NodeDrawable.h"

void MelonRenderer::NodeDrawable::Tick(PipelineData& pipelineData, const mat4& parentMat)
{
	m_transform = m_transformationMat * parentMat;

	m_drawable->Tick(pipelineData);

	for (auto child : m_children)
	{
		child->Tick(pipelineData, m_transform);
	}
}

void MelonRenderer::NodeDrawable::Tick(PipelineData& pipelineData)
{
	for (auto child : m_children)
	{
		child->Tick(pipelineData, m_transformationMat);
	}

	m_transform = m_transformationMat;
	m_drawable->Tick(pipelineData);
}

void MelonRenderer::NodeDrawable::SetDrawable(Drawable* drawable)
{
	m_drawable = drawable;
}

void MelonRenderer::NodeDrawable::SetMaterialIndices(uint32_t materialIndice)
{
	//TODO: replace material functionality
}
