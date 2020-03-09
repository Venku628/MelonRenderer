#pragma once

#include "Basics.h"

struct Vertex {
	float posX, posY, posZ; 
	float normalX, normalY, normalZ;
	float u, v;

	//needed for unordnered map
	bool operator==(const Vertex& other) const {
		return posX == other.posX && posY == other.posY && posZ == other.posZ &&
			normalX == other.normalX && normalY == other.normalY && normalZ == other.normalZ&&
			u == other.u && v == other.v ;
	}
};

//TODO: quickly done, research alternatives
namespace std {
	template <>
	struct hash<Vertex>
	{
		std::size_t operator()(const Vertex& k) const
		{
			size_t h1 = std::hash<float>()(k.posX);
			size_t h2 = std::hash<float>()(k.posY);
			size_t h3 = std::hash<float>()(k.posZ);
			size_t h4 = std::hash<float>()(k.u);
			size_t h5 = std::hash<float>()(k.v);

			return h1 ^ (h2 << 1) ^ (h3 << 2) ^ (h4 << 3 ) ^ (h5 << 4);
		}
	};
}


static const Vertex cube_vertex_data[] = {
	//front
	{-1.f, -1.f, 1.f, 0.f, 0.f, 1.f, 1.f, 0.f},
	{ 1.f, -1.f, 1.f, 0.f, 0.f, 1.f, 0.f, 0.f},
	{ 1.f, 1.f, 1.f, 0.f, 0.f, 1.f, 0.f, 1.f},
	{ -1.f, 1.f, 1.f, 0.f, 0.f, 1.f, 1.f, 1.f},
	//back
	{ -1.f, 1.f, -1.f, 0.f, 0.f, -1.f, 1.f, 0.f},
	{ 1.f, 1.f, -1.f, 0.f, 0.f, -1.f, 0.f, 0.f},
	{ 1.f, -1.f, -1.f, 0.f, 0.f, -1.f, 0.f, 1.f},
	{ -1.f, -1.f, -1.f, 0.f, 0.f, -1.f, 1.f, 1.f},
	//right
	{ 1.f, -1.f, 1.f, 1.f, 0.f, 0.f, 1.f, 0.f},
	{ 1.f, -1.f, -1.f, 1.f, 0.f, 0.f, 0.f, 0.f},
	{ 1.f, 1.f, -1.f, 1.f, 0.f, 0.f, 0.f, 1.f},
	{ 1.f, 1.f, 1.f, 1.f, 0.f, 0.f, 1.f, 1.f},
	//left
	{ -1.f, 1.f, 1.f, -1.f, 0.f, 0.f, 1.f, 0.f},
	{ -1.f, 1.f, -1.f, -1.f, 0.f, 0.f, 0.f, 0.f},
	{ -1.f, -1.f, -1.f, -1.f, 0.f, 0.f, 0.f, 1.f},
	{ -1.f, -1.f, 1.f, -1.f, 0.f, 0.f, 1.f, 1.f},
	//top
	{ -1.f, 1.f, 1.f, 0.f, 1.f, 0.f, 1.f, 0.f},
	{ 1.f, 1.f, 1.f, 0.f, 1.f, 0.f, 0.f, 0.f},
	{ 1.f, 1.f, -1.f, 0.f, 1.f, 0.f, 0.f, 1.f},
	{ -1.f, 1.f, -1.f, 0.f, 1.f, 0.f, 1.f, 1.f},
	//bottom
	{ -1.f, -1.f, -1.f, 0.f, -1.f, 0.f, 1.f, 0.f},
	{ 1.f, -1.f, -1.f, 0.f, -1.f, 0.f, 0.f, 0.f},
	{ 1.f, -1.f, 1.f, 0.f, -1.f, 0.f, 0.f, 1.f},
	{ -1.f, -1.f, 1.f, 0.f, -1.f, 0.f, 1.f, 1.f}
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
