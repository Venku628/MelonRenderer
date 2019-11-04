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