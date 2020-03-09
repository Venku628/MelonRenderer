#include "NodeCamera.h"

mat4 MelonRenderer::NodeCamera::CalculateWorldTransform(const mat4& parentMat)
{
	return m_transformationMat * parentMat;
}

void MelonRenderer::NodeCamera::SetCamera(Camera* camera)
{
	m_camera = camera;
	//m_transformationMat = m_camera->m_modelViewProjection;
}
