#pragma once

#include "Basics.h"
#include "cube.h"
#include "DeviceMemoryManager.h"
#include <unordered_map>

namespace MelonRenderer {

	typedef uint32_t MeshIndex;

	struct DrawableInstance
	{
		mat4 m_transformation;
		mat4 m_transformationInverseTranspose;
		uint32_t m_drawableIndex;
		uint32_t m_textureOffset;
		//uint8_t alignmentPadding[8];
	};

	struct WaveFrontMaterial
	{
		vec3  ambient = vec3(1.f, 1.f, 1.f);
		vec3  diffuse = vec3(0.8f, 0.8f, 0.8f);
		vec3  specular = vec3(0.5f, 0.5f, 0.5f);
		vec3  transmittance = vec3(0.f, 0.f, 0.f);
		vec3  emission = vec3(0.f, 0.f, 0.f);
		float shininess = 225.f;
		float indexOfRefraction = 1.f;       // index of refraction
		float dissolve = 1.f;  // 1 == opaque; 0 == fully transparent
		int   illum = 2;     // illumination model (see http://www.fileformat.info/format/material/)
		int   textureId = 0;
	};

	class Drawable
	{
	public:
		bool LoadMeshData(DeviceMemoryManager& memoryManager, const std::string& path);

		bool Init(DeviceMemoryManager& memoryManager);
		bool Init(DeviceMemoryManager& memoryManager, const std::string& path);
		//void Tick(PipelineData& pipelineData);
		void Fini();

	protected:
		std::vector<Vertex> m_vertices;
		VkBuffer m_vertexBuffer;
		VkDeviceMemory m_vertexBufferMemory;
		uint32_t m_vertexCount;

		std::vector<uint32_t> m_indices;
		VkBuffer m_indexBuffer;
		VkDeviceMemory m_indexBufferMemory;
		uint32_t m_indexCount;

		std::vector<WaveFrontMaterial> m_materials;
		VkBuffer m_materialBuffer;
		VkDeviceMemory m_materialBufferMemory;

		friend class Pipeline;
		friend class PipelineRasterization;
		friend class PipelineRaytracing;
	};
}