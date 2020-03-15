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

		if (materials.size())
		{
			for (int i = 0; i < materials.size(); i++)
			{
				WaveFrontMaterial material = {};
				material.ambient = glm::make_vec3(materials[i].ambient);
				material.diffuse = glm::make_vec3(materials[i].diffuse);
				material.specular = glm::make_vec3(materials[i].specular);
				material.transmittance = glm::make_vec3(materials[i].transmittance);
				material.emission = glm::make_vec3(materials[i].emission);
				material.shininess = materials[i].shininess;
				material.ior = materials[i].ior;
				material.dissolve = materials[i].dissolve;
				material.illum = materials[i].illum;
				//TODO: do lookup if texture name exists already, map

				m_materials.emplace_back(material);
			}
		}
		else
		{
			//default material
			WaveFrontMaterial material = {};
			m_materials.emplace_back(material);
		}

		//to make use of indices, we need to ignore duplicates
		std::unordered_map<Vertex, uint32_t> uniqueVertices;

		for (const auto& shape : shapes) {
			uint32_t faceID = 0;
			uint32_t indexCount = 0;
			for (const auto& index : shape.mesh.indices) {
				Vertex vertex = {};

				vertex.posX = attributes.vertices[3 * index.vertex_index];
				vertex.posY = attributes.vertices[3 * index.vertex_index + 1];
				vertex.posZ = attributes.vertices[3 * index.vertex_index + 2];

				if (!attributes.normals.empty())
				{
					vertex.normalX = attributes.normals[3 * index.normal_index];
					vertex.normalY = attributes.normals[3 * index.normal_index + 1];
					vertex.normalZ = attributes.normals[3 * index.normal_index + 2];
				}

				if (!attributes.texcoords.empty() && index.texcoord_index >= 0)
				{
					vertex.u = attributes.texcoords[2 * index.texcoord_index + 0];
					vertex.v = attributes.texcoords[2 * index.texcoord_index + 1]; //flip the coord with 1.f -   ?
				}
				else
				{
					vertex.u = 0.f;
					vertex.v = 0.f;
				}

				vertex.matID = shape.mesh.material_ids[faceID];
				indexCount++;
				if (indexCount == 3) //every 3 vertices, one face
				{
					faceID++;
					indexCount = 0;
				}

				if (uniqueVertices.count(vertex) == 0) {
					uniqueVertices[vertex] = static_cast<uint32_t>(m_vertices.size());
					m_vertices.push_back(vertex);
				}

				m_indices.push_back(uniqueVertices[vertex]);
			}
		}

		// some objs do not come with normals
		if (attributes.normals.empty())
		{
			for (size_t i = 0; i < m_indices.size(); i += 3)
			{
				Vertex& v0 = m_vertices[m_indices[i + 0]];
				Vertex& v1 = m_vertices[m_indices[i + 1]];
				Vertex& v2 = m_vertices[m_indices[i + 2]];

				glm::vec3 normal = glm::normalize(glm::cross(
					(vec3(v1.posX, v1.posY, v1.posZ) - vec3(v0.posX, v0.posY, v0.posZ)), 
					(vec3(v2.posX, v2.posY, v2.posZ) - vec3(v0.posX, v0.posY, v0.posZ))));


				v0.normalX = normal.x;
				v0.normalY = normal.y;
				v0.normalZ = normal.z;

				v1.normalX = normal.x;
				v1.normalY = normal.y;
				v1.normalZ = normal.z;

				v2.normalX = normal.x;
				v2.normalY = normal.y;
				v2.normalZ = normal.z;
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

		//default cube material
		WaveFrontMaterial material = {};
		material.textureId = 1;
		m_materials.emplace_back(material);
		uint32_t materialBuffersize = m_materials.size() * sizeof(WaveFrontMaterial);
		if (!memoryManager.CreateOptimalBuffer(m_materialBuffer, m_materialBufferMemory, m_materials.data(), materialBuffersize,
			VK_BUFFER_USAGE_STORAGE_BUFFER_BIT))
		{
			Logger::Log("Could not create material buffer.");
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

		uint32_t indexBufferSize = sizeof(uint32_t) * m_indices.size();
		if (!memoryManager.CreateOptimalBuffer(m_indexBuffer, m_indexBufferMemory, m_indices.data(), indexBufferSize,
			VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT))
		{
			Logger::Log("Could not create index buffer.");
			return false;
		}

		uint32_t materialBuffersize = m_materials.size() * sizeof(WaveFrontMaterial);
		if (!memoryManager.CreateOptimalBuffer(m_materialBuffer, m_materialBufferMemory, m_materials.data(), materialBuffersize,
			VK_BUFFER_USAGE_STORAGE_BUFFER_BIT))
		{
			Logger::Log("Could not create material buffer.");
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