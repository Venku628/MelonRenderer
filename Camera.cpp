#include "Camera.h"

bool MelonRenderer::Camera::Init(const DeviceMemoryManager& memoryManager)
{
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
	m_uniformBufferDescriptor.range = sizeof(CameraMatrices);

	return true;
}

bool MelonRenderer::Camera::Tick(CameraMatrices& cameraMatrices)
{
	m_cameraMatrices = cameraMatrices;

	void* pData;
	VkResult result = vkMapMemory(Device::Get().m_device, m_uniformBufferMemory, 0, sizeof(cameraMatrices), 0, (void**)&pData);
	if (result != VK_SUCCESS)
	{
		Logger::Log("Could not map memory for uniform buffer memory.");
		return false;
	}
	memcpy(pData, &m_cameraMatrices, sizeof(m_cameraMatrices));
	vkUnmapMemory(Device::Get().m_device, m_uniformBufferMemory); //immediatley unmap because of limited page table for gpu+cpu adresses

	return true;
}

VkDescriptorBufferInfo* MelonRenderer::Camera::GetCameraDescriptor()
{
	return &m_uniformBufferDescriptor;
}
