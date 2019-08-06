#pragma once

#include "Renderer.h"

int main(int argc, char* argv[]) { 

	MelonRenderer::Logger::Get().Print();

	MelonRenderer::Renderer test;
	test.Init();

	MelonRenderer::Logger::Log("Hello!");
}
