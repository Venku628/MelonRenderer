#include "NodeDrawable.h"

void MelonRenderer::NodeDrawable::Tick(PipelineData* pipelineData, const mat4& parentMat)
{
	m_objectData.transformMatrix = m_transformationMat * parentMat;

	m_drawable->Tick(*pipelineData->m_commandBuffer, *pipelineData->m_pipelineLayout, m_objectData);

	for (auto child : m_children)
	{
		child->Tick(pipelineData, m_objectData.transformMatrix);
	}
}

void MelonRenderer::NodeDrawable::Tick(PipelineData* pipelineData)
{
	for (auto child : m_children)
	{
		child->Tick(pipelineData, m_transformationMat);
	}

	m_objectData.transformMatrix = m_transformationMat;
	m_drawable->Tick(*pipelineData->m_commandBuffer, *pipelineData->m_pipelineLayout, m_objectData);
}

void MelonRenderer::NodeDrawable::SetDrawable(Drawable* drawable)
{
	m_drawable = drawable;
}

void MelonRenderer::NodeDrawable::SetMaterialIndices(uint32_t materialIndice)
{
	m_objectData.materialIndices[0] = materialIndice;
}
