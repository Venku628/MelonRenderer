#include "Camera.h"

bool MelonRenderer::Camera::Init(DeviceMemoryManager& memoryManager)
{
	m_memoryManager = &memoryManager;

	uint32_t uniformBufferSize = sizeof(CameraMatrices);
	if (!memoryManager.CreateBuffer(uniformBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_uniformBuffer, m_uniformBufferMemory))
	{
		Logger::Log("Could not create uniform buffer.");
		return false;
	}

	m_cameraMatrices.projection = glm::perspective(glm::radians(45.0f), 1.0f, 0.1f, 100.0f);
	m_cameraMatrices.view = glm::lookAt(vec3(-5, 3, -10),
		vec3(0, 0, 0),
		vec3(0, -1, 0));
	m_cameraMatrices.projectionInverse = glm::inverse(m_cameraMatrices.projection);
	m_cameraMatrices.viewInverse = glm::inverse(m_cameraMatrices.view);


	// Vulkan clip space has inverted Y and half Z.
	mat4 clip = glm::mat4(1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f, //had y at -1 before but did not work as intended? correct result now 
		0.0f, 0.0f, 0.5f, 0.f,
		0.0f, 0.0f, 0.5f, 1.0f);

	Tick(m_cameraMatrices);

	m_uniformBufferDescriptor.buffer = m_uniformBuffer;
	m_uniformBufferDescriptor.offset = 0;
	m_uniformBufferDescriptor.range = VK_WHOLE_SIZE;

	return true;
}

bool MelonRenderer::Camera::Tick(CameraMatrices& cameraMatrices)
{
	m_cameraMatrices = cameraMatrices;

	if (!m_memoryManager->CopyDataToMemory(m_uniformBufferMemory, &m_cameraMatrices, sizeof(m_cameraMatrices)))
	{
		Logger::Log("Could not copy camera data to uniform buffer.");
		return false;
	}

	return true;
}

bool MelonRenderer::Camera::Tick()
{
	ImGui::Begin("Camera");
	vec3 cameraPositionVec, cameraDirectionVec;
	static float cameraPosition[3] = { -7.5f, 3.f, -12.f };
	ImGui::InputFloat3("camera position", cameraPosition);
	cameraPositionVec.x = cameraPosition[0];
	cameraPositionVec.y = cameraPosition[1];
	cameraPositionVec.z = cameraPosition[2];
	static float cameraDirection[3] = { 0.25f, -0.15f, 0.525f };
	ImGui::SliderFloat3("camera direction", cameraDirection, -1.f, 1.f);
	cameraDirectionVec.x = cameraDirection[0];
	cameraDirectionVec.y = cameraDirection[1];
	cameraDirectionVec.z = cameraDirection[2];
	cameraDirectionVec = glm::normalize(cameraDirectionVec);
	ImGui::End();

	const vec3 worldUp = glm::vec3(0.0f, -1.0f, 0.0f);
	vec3 cameraUp = glm::cross(cameraDirectionVec, glm::normalize(glm::cross(worldUp, cameraDirectionVec)));

	m_cameraMatrices.view = glm::lookAt(cameraPositionVec, cameraPositionVec + cameraDirectionVec, cameraUp);
	m_cameraMatrices.viewInverse = glm::inverse(m_cameraMatrices.view);
	
	if (!m_memoryManager->CopyDataToMemory(m_uniformBufferMemory, &m_cameraMatrices, sizeof(m_cameraMatrices)))
	{
		Logger::Log("Could not copy camera data to uniform buffer.");
		return false;
	}

	return true;
}

VkDescriptorBufferInfo* MelonRenderer::Camera::GetCameraDescriptor()
{
	return &m_uniformBufferDescriptor;
}
