#pragma once

#include "Basics.h"
#include "DeviceMemoryManager.h"
#include "imgui/imgui.h"
#include <glfw3.h>

namespace MelonRenderer
{
	struct CameraMatrices
	{
		mat4 view;
		mat4 projection;
		mat4 viewInverse;
		mat4 projectionInverse;
	};

	class Camera
	{
	public:
		bool Init(DeviceMemoryManager& memoryManager);
		bool Tick(CameraMatrices& cameraMatrices);
		bool Tick(GLFWwindow* glfwWindow);

		VkDescriptorBufferInfo* GetCameraDescriptor();

	protected:

		CameraMatrices m_cameraMatrices;
		const vec3 worldUp = glm::vec3(0.0f, -1.0f, 0.0f);
		vec3 m_cameraPosition = vec3(-7.5f, 3.f, 5.f);
		vec3 m_cameraDirection;

		VkBuffer m_uniformBuffer;
		VkDeviceMemory m_uniformBufferMemory;
		VkDescriptorBufferInfo m_uniformBufferDescriptor;

		DeviceMemoryManager* m_memoryManager;
	};
}
