#pragma once

struct Vertex {
	float posX, posY, posZ;  // Position data
	float r, g, b;              // Color
};

static const Vertex g_vb_solid_face_colors_Data[] = {
	// red face
	{-1.f, -1.f, 1.f, 1.f, 0.f, 0.f},
	{-1.f, 1.f, 1.f, 1.f, 0.f, 0.f},
	{1.f, -1.f, 1.f, 1.f, 0.f, 0.f},
	{1.f, -1.f, 1.f, 1.f, 0.f, 0.f},
	{-1.f, 1.f, 1.f, 1.f, 0.f, 0.f},
	{1.f, 1.f, 1.f, 1.f, 0.f, 0.f},
	// green face
	{-1.f, -1.f, -1.f, 0.f, 1.f, 0.f},
	{1.f, -1.f, -1.f, 0.f, 1.f, 0.f},
	{-1.f, 1.f, -1.f, 0.f, 1.f, 0.f},
	{-1.f, 1.f, -1.f, 0.f, 1.f, 0.f},
	{1.f, -1.f, -1.f, 0.f, 1.f, 0.f},
	{1.f, 1.f, -1.f, 0.f, 1.f, 0.f},
	// blue face
	{-1.f, 1.f, 1.f, 0.f, 0.f, 1.f},
	{-1.f, -1.f, 1.f, 0.f, 0.f, 1.f},
	{-1.f, 1.f, -1.f, 0.f, 0.f, 1.f},
	{-1.f, 1.f, -1.f, 0.f, 0.f, 1.f},
	{-1.f, -1.f, 1.f, 0.f, 0.f, 1.f},
	{-1.f, -1.f, -1.f, 0.f, 0.f, 1.f},
	// yellow face
	{1.f, 1.f, 1.f, 1.f, 1.f, 0.f},
	{1.f, 1.f, -1.f, 1.f, 1.f, 0.f},
	{1.f, -1.f, 1.f, 1.f, 1.f, 0.f},
	{1.f, -1.f, 1.f, 1.f, 1.f, 0.f},
	{1.f, 1.f, -1.f, 1.f, 1.f, 0.f},
	{1.f, -1.f, -1.f, 1.f, 1.f, 0.f},
	// magenta face
	{1.f, 1.f, 1.f, 1.f, 0.f, 1.f},
	{-1.f, 1.f, 1.f, 1.f, 0.f, 1.f},
	{1.f, 1.f, -1.f, 1.f, 0.f, 1.f},
	{1.f, 1.f, -1.f, 1.f, 0.f, 1.f},
	{-1.f, 1.f, 1.f, 1.f, 0.f, 1.f},
	{-1.f, 1.f, -1.f, 1.f, 0.f, 1.f},
	// cyan face
	{1.f, -1.f, 1.f, 0.f, 1.f, 1.f},
	{1.f, -1.f, -1.f, 0.f, 1.f, 1.f},
	{-1.f, -1.f, 1.f, 0.f, 1.f, 1.f},
	{-1.f, -1.f, 1.f, 0.f, 1.f, 1.f},
	{1.f, -1.f, -1.f, 0.f, 1.f, 1.f},
	{-1.f, -1.f, -1.f, 0.f, 1.f, 1.f}
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