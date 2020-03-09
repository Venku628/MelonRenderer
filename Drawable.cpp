#include "Drawable.h"
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

namespace MelonRenderer {
	bool Drawable::LoadMeshData(std::string path)
	{
		tinyobj::attrib_t attributes;
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;
		std::string warnings, errors;

		if (!tinyobj::LoadObj(&attributes, &shapes, &materials, &warnings, &errors, path.c_str(), "models/")) 
		{
			Logger::Log(warnings + errors);
			//return false;
		}

		if (materials.size() > 0)
		{
			m_material.ambient = glm::make_vec3(materials[0].ambient);
			m_material.diffuse = glm::make_vec3(materials[0].diffuse);
			m_material.specular = glm::make_vec3(materials[0].specular);
			m_material.transmittance = glm::make_vec3(materials[0].transmittance);
			m_material.emission = glm::make_vec3(materials[0].emission);
			m_material.shininess = materials[0].shininess;
			m_material.ior = materials[0].ior;
			m_material.dissolve = materials[0].dissolve;
			m_material.illum = materials[0].illum;
			//textureID elsewhere
			//TODO: do lookup if texture name exists already, map
		}

		//to make use of indices, we need to ignore duplicates
		std::unordered_map<Vertex, uint32_t> uniqueVertices;

		for (const auto& shape : shapes) {
			for (const auto& index : shape.mesh.indices) {
				Vertex vertex = {};

				vertex.posX = attributes.vertices[3 * index.vertex_index];
				vertex.posY = attributes.vertices[3 * index.vertex_index + 1];
				vertex.posZ = attributes.vertices[3 * index.vertex_index + 2];

				vertex.normalX = attributes.normals[3 * index.normal_index];
				vertex.normalY = attributes.normals[3 * index.normal_index + 1];
				vertex.normalZ = attributes.normals[3 * index.normal_index + 2];

				vertex.u = attributes.texcoords[2 * index.texcoord_index + 0];
				vertex.v = attributes.texcoords[2 * index.texcoord_index + 1]; //flip the coord with 1.f -   ?

				if (uniqueVertices.count(vertex) == 0) {
					uniqueVertices[vertex] = static_cast<uint32_t>(m_vertices.size());
					m_vertices.push_back(vertex);
				}

				m_indices.push_back(uniqueVertices[vertex]);
			}
		}

		return true;
	}

	bool Drawable::Init(DeviceMemoryManager& memoryManager)
	{
		uint32_t vertexBufferSize = sizeof(cube_vertex_data);
		if (!memoryManager.CreateOptimalBuffer(m_vertexBuffer, m_vertexBufferMemory, cube_vertex_data, vertexBufferSize, 
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT))
		{
			Logger::Log("Could not create vertex buffer.");
			return false;
		}

		uint32_t indexBufferSize = sizeof(cube_index_data);
		if (!memoryManager.CreateOptimalBuffer(m_indexBuffer, m_indexBufferMemory, cube_index_data, indexBufferSize, 
			VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT))
		{
			Logger::Log("Could not create index buffer.");
			return false;
		}

		m_vertexCount = sizeof(cube_vertex_data) / sizeof(Vertex);
		m_indexCount = sizeof(cube_index_data) / sizeof(uint32_t);

		return true;
	}

	bool Drawable::Init(DeviceMemoryManager& memoryManager, std::string path)
	{
		LoadMeshData(path);

		uint32_t vertexBufferSize = sizeof(Vertex) * m_vertices.size();
		if (!memoryManager.CreateOptimalBuffer(m_vertexBuffer, m_vertexBufferMemory, m_vertices.data(), vertexBufferSize,
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT))
		{
			Logger::Log("Could not create vertex buffer.");
			return false;
		}

		//TODO: make drawable attribute
		uint32_t indexBufferSize = sizeof(uint32_t) * m_indices.size();
		if (!memoryManager.CreateOptimalBuffer(m_indexBuffer, m_indexBufferMemory, m_indices.data(), indexBufferSize,
			VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT))
		{
			Logger::Log("Could not create index buffer.");
			return false;
		}

		m_vertexCount = m_vertices.size();
		m_indexCount = m_indices.size();

		return true;
	}

	/*
	void Drawable::Tick(PipelineData& pipelineData)
	{
		const VkDeviceSize offsets[1] = { 0 };
		vkCmdBindVertexBuffers(*pipelineData.m_commandBuffer, 0, 1, &m_vertexBuffer, offsets);
		vkCmdBindIndexBuffer(*pipelineData.m_commandBuffer, m_indexBuffer, 0, VK_INDEX_TYPE_UINT32);

		//TODO: do once for every instance
		vkCmdBindDescriptorSets(*pipelineData.m_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, *pipelineData.m_pipelineLayout, 0, 
			pipelineData.m_descriptorSets->size(), pipelineData.m_descriptorSets->data(),
			1, pipelineData.m_transformOffset);

		vkCmdDrawIndexed(*pipelineData.m_commandBuffer, sizeof(cube_index_data) / sizeof(uint32_t), 1, 0, 0, 0);

		*pipelineData.m_transformOffset += pipelineData.m_alignment;
	}
	*/
}