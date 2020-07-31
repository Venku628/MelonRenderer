#pragma once

#include "Basics.h"


struct Vertex {
	float posX, posY, posZ;
	float normalX, normalY, normalZ;
	float u, v;
	uint32_t matID = 0;

	//needed for unordnered map
	bool operator==(const Vertex& other) const {
		return posX == other.posX && posY == other.posY && posZ == other.posZ &&
			normalX == other.normalX && normalY == other.normalY && normalZ == other.normalZ &&
			u == other.u && v == other.v;
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

			return h1 ^ (h2 << 1) ^ (h3 << 2) ^ (h4 << 3) ^ (h5 << 4);
		}
	};
}