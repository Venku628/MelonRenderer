#pragma once

#include "Basics.h"

struct VertexNeu {
	float posX, posY, posZ;
	float normalX, normalY, normalZ;
	float tangentX, tangentY, tangentZ;
	float bitangentX, bitangentY, bitangentZ;
	float texU, texV;
};

class Drawable
{
public:
	void CreateVertexBuffer();

protected:
	VkBuffer m_vertexBuffer;
	VkDeviceMemory m_vertexBufferMemory;
	VkVertexInputBindingDescription m_vertexInputBinding;

};

