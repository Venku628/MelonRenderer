#pragma once

#include "Renderer.h"

int main(int argc, char* argv[]) { 

	MelonRenderer::Logger::Get().Print();
	MelonRenderer::Renderer instance;

	instance.Init();
	instance.Loop();
	instance.Fini();

	return 0;
}
