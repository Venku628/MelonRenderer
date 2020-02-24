#pragma once

#include "Basics.h"
#include "DeviceMemoryManager.h"

namespace MelonRenderer
{
	class Camera
	{
	public:
		bool Init(const DeviceMemoryManager& memoryManager);
		bool Tick(mat4& mvpMat);

		VkDescriptorBufferInfo* GetCameraDescriptor();

		mat4 m_modelViewProjection;

	protected:
		VkBuffer m_uniformBuffer;
		VkDeviceMemory m_uniformBufferMemory;
		VkDescriptorBufferInfo m_uniformBufferDescriptor;
	};
}
