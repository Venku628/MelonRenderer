#pragma once

#include "Renderer.h"

int main(int argc, char* argv[]) { 

	MelonRenderer::Logger::Get().Print();

	MelonRenderer::Window window;
	MelonRenderer::Renderer test;

	window.Init("Debug");
	test.Init(window.m_windowHandle);

	while (true)
	{
		window.Tick(test);
		MelonRenderer::Logger::Get().Print();
	}
	
	test.Fini();
}
