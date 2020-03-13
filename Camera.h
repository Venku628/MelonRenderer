#pragma once

#include "Basics.h"
#include "DeviceMemoryManager.h"
#include "imgui/imgui.h"

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
		bool Tick();

		VkDescriptorBufferInfo* GetCameraDescriptor();

		CameraMatrices m_cameraMatrices;

	protected:
		VkBuffer m_uniformBuffer;
		VkDeviceMemory m_uniformBufferMemory;
		VkDescriptorBufferInfo m_uniformBufferDescriptor;

		DeviceMemoryManager* m_memoryManager;
	};
}
