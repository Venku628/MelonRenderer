#pragma once

#include "Basics.h"
#include "DeviceMemoryManager.h"

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
		bool Init(const DeviceMemoryManager& memoryManager);
		bool Tick(CameraMatrices& cameraMatrices);

		VkDescriptorBufferInfo* GetCameraDescriptor();

		CameraMatrices m_cameraMatrices;

	protected:
		VkBuffer m_uniformBuffer;
		VkDeviceMemory m_uniformBufferMemory;
		VkDescriptorBufferInfo m_uniformBufferDescriptor;
	};
}
