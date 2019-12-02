#pragma once

#include "Basics.h"

struct Vertex {
	float posX, posY, posZ; 
	float r, g, b;              //debug
	float u, v;
};

typedef uint32_t MeshIndex;

struct ObjectData {
	mat4 transformMatrix;
	uint32_t textureIndex;
};

#define ColorVertex(x, y, z) (x), (y), (z), 1.f, 0.f, 0.f


static const Vertex cube_vertex_data[] = {
	//front
	{ColorVertex(-1.f, -1.f, 1.f), 1.f, 0.f},
	{ColorVertex(1.f, -1.f, 1.f), 0.f, 0.f},
	{ColorVertex(1.f, 1.f, 1.f), 0.f, 1.f},
	{ColorVertex(-1.f, 1.f, 1.f), 1.f, 1.f},
	//back
	{ColorVertex(-1.f, 1.f, -1.f), 1.f, 0.f},
	{ColorVertex(1.f, 1.f, -1.f), 0.f, 0.f},
	{ColorVertex(1.f, -1.f, -1.f), 0.f, 1.f},
	{ColorVertex(-1.f, -1.f, -1.f), 1.f, 1.f},
	//right
	{ColorVertex(1.f, -1.f, 1.f), 1.f, 0.f},
	{ColorVertex(1.f, -1.f, -1.f), 0.f, 0.f},
	{ColorVertex(1.f, 1.f, -1.f), 0.f, 1.f},
	{ColorVertex(1.f, 1.f, 1.f), 1.f, 1.f},
	//left
	{ColorVertex(-1.f, 1.f, 1.f), 1.f, 0.f},
	{ColorVertex(-1.f, 1.f, -1.f), 0.f, 0.f},
	{ColorVertex(-1.f, -1.f, -1.f), 0.f, 1.f},
	{ColorVertex(-1.f, -1.f, 1.f), 1.f, 1.f},
	//top
	{ColorVertex(-1.f, 1.f, 1.f), 1.f, 0.f},
	{ColorVertex(1.f, 1.f, 1.f), 0.f, 0.f},
	{ColorVertex(1.f, 1.f, -1.f), 0.f, 1.f},
	{ColorVertex(-1.f, 1.f, -1.f), 1.f, 1.f},
	//bottom
	{ColorVertex(-1.f, -1.f, -1.f), 1.f, 0.f},
	{ColorVertex(1.f, -1.f, -1.f), 0.f, 0.f},
	{ColorVertex(1.f, -1.f, 1.f), 0.f, 1.f},
	{ColorVertex(-1.f, -1.f, 1.f), 1.f, 1.f}
};

static const uint32_t cube_index_data[] = {
	//front
	0, 1, 2,
	0, 2, 3,
	//back
	7, 5, 6,
	7, 4, 5,
	//right
	8, 9, 10,
	8, 10, 11,
	//left
	15, 12, 13,
	15, 13, 14,
	//top
	16, 17, 18,
	16, 18, 19,
	//bottom
	23, 21, 22,
	23, 20, 21
};
