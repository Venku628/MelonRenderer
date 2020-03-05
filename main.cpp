#pragma once

#include "Renderer.h"

int main(int argc, char* argv[]) { 

	MelonRenderer::Logger::Get().Print();
	MelonRenderer::Renderer instance;

	MelonRenderer::Logger::Get().SetModeImmediate(true);

	instance.Init();
	instance.Loop();
	instance.Fini();

	return 0;
}
