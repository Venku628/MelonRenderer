#include "NodeCamera.h"

void MelonRenderer::NodeCamera::Tick(PipelineData& pipelineData, const mat4& parentMat)
{
	mat4 actualTransformationMat = m_transformationMat * parentMat;

	m_camera->Tick(actualTransformationMat);

	for (auto child : m_children)
	{
		child->Tick(pipelineData, actualTransformationMat);
	}
}

void MelonRenderer::NodeCamera::Tick(PipelineData& pipelineData)
{
	for (auto child : m_children)
	{
		child->Tick(pipelineData, m_transformationMat);
	}

	m_camera->Tick(m_transformationMat);
}

void MelonRenderer::NodeCamera::SetCamera(Camera* camera)
{
	m_camera = camera;
	m_transformationMat = m_camera->m_modelViewProjection;
}
