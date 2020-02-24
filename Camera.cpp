#include "Camera.h"

bool MelonRenderer::Camera::Init(const DeviceMemoryManager& memoryManager)
{
	uint32_t uniformBufferSize = sizeof(m_modelViewProjection);
	if (!memoryManager.CreateBuffer(uniformBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_uniformBuffer, m_uniformBufferMemory))
	{
		Logger::Log("Could not create uniform buffer.");
		return false;
	}

	mat4 projection = glm::perspective(glm::radians(45.0f), 1.0f, 0.1f, 100.0f);
	mat4 view = glm::lookAt(vec3(-5, 3, -10),
		vec3(0, 0, 0),
		vec3(0, -1, 0));

	// Vulkan clip space has inverted Y and half Z.
	mat4 clip = glm::mat4(1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f, //had y at -1 before but did not work as intended? correct result now 
		0.0f, 0.0f, 0.5f, 0.f,
		0.0f, 0.0f, 0.5f, 1.0f);
	projection = clip * projection * view;

	Tick(projection);

	m_uniformBufferDescriptor.buffer = m_uniformBuffer;
	m_uniformBufferDescriptor.offset = 0;
	m_uniformBufferDescriptor.range = sizeof(m_modelViewProjection);

	return true;
}

bool MelonRenderer::Camera::Tick(mat4& mvpMat)
{
	m_modelViewProjection = mvpMat;

	void* pData;
	VkResult result = vkMapMemory(Device::Get().m_device, m_uniformBufferMemory, 0, sizeof(mvpMat), 0, (void**)&pData);
	if (result != VK_SUCCESS)
	{
		Logger::Log("Could not map memory for uniform buffer memory.");
		return false;
	}
	memcpy(pData, &m_modelViewProjection, sizeof(m_modelViewProjection));
	vkUnmapMemory(Device::Get().m_device, m_uniformBufferMemory); //immediatley unmap because of limited page table for gpu+cpu adresses

	return true;
}

VkDescriptorBufferInfo* MelonRenderer::Camera::GetCameraDescriptor()
{
	return &m_uniformBufferDescriptor;
}
