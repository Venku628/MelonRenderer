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
		0.0f, 1.0f, 0.0f, 0.0f, //had y at -1 before, as per vulkan samples but did not work as intended? correct result now 
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

bool MelonRenderer::Camera::Tick(GLFWwindow* glfwWindow)
{
	ImGui::Begin("Camera");
	static float cameraPosition[3] = { m_cameraPosition.x, m_cameraPosition.y, m_cameraPosition.z };
	cameraPosition[0] = m_cameraPosition.x;
	cameraPosition[1] = m_cameraPosition.y;
	cameraPosition[2] = m_cameraPosition.z;
	ImGui::InputFloat3("camera position", cameraPosition);
	m_cameraPosition.x = cameraPosition[0];
	m_cameraPosition.y = cameraPosition[1];
	m_cameraPosition.z = cameraPosition[2];
	static float cameraDirection[3] = { -0.5f, -0.2f, -0.5f };
	ImGui::SliderFloat3("camera direction", cameraDirection, -1.f, 1.f);
	m_cameraDirection.x = cameraDirection[0];
	m_cameraDirection.y = cameraDirection[1];
	m_cameraDirection.z = cameraDirection[2];
	m_cameraDirection = glm::normalize(m_cameraDirection);
	ImGui::End();

	//https://learnopengl.com/Getting-started/Camera
	vec3 cameraUp = glm::cross(m_cameraDirection, glm::normalize(glm::cross(worldUp, m_cameraDirection)));

	const float cameraSpeed = 0.05f; // adjust accordingly
	if (glfwGetKey(glfwWindow, GLFW_KEY_W) == GLFW_PRESS)
		m_cameraPosition += cameraSpeed * m_cameraDirection;
	if (glfwGetKey(glfwWindow, GLFW_KEY_S) == GLFW_PRESS)
		m_cameraPosition -= cameraSpeed * m_cameraDirection;
	if (glfwGetKey(glfwWindow, GLFW_KEY_A) == GLFW_PRESS)
		m_cameraPosition -= glm::normalize(glm::cross(m_cameraDirection, cameraUp)) * cameraSpeed;
	if (glfwGetKey(glfwWindow, GLFW_KEY_D) == GLFW_PRESS)
		m_cameraPosition += glm::normalize(glm::cross(m_cameraDirection, cameraUp)) * cameraSpeed;


	m_cameraMatrices.view = glm::lookAt(m_cameraPosition, m_cameraPosition + m_cameraDirection, cameraUp);
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
