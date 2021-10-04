#pragma once

#include "Kablunk/Core/Core.h"
#include "Kablunk/Core/Application.h"

#ifdef KB_PLATFORM_WINDOWS


extern Kablunk::Application* Kablunk::CreateApplication();

#ifdef KB_PLATFORM_WINDOWS
#	ifndef PREFER_HIGH_PERFORMANCE_GPU
#	define PREFER_HIGH_PERFORMANCE_GPU
// Auto select high performance gpu
extern "C" {
	_declspec(dllexport) DWORD NvOptimusEnablement = 1;
	_declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}
#	endif
#endif

int main(int argc, char** argv) 
{
	Kablunk::InitCore();

	KB_BEGIN_SESSION("Startup", "KablunkProfile-Startup.json");
	Kablunk::Application* app = Kablunk::CreateApplication();
	KB_END_SESSION();

	KB_BEGIN_SESSION("Runtime", "KablunkProfile-Runtime.json");
	app->Run();
	KB_END_SESSION();

	KB_BEGIN_SESSION("Shutdown", "KablunkProfile-Shutdown.json");
	delete app;
	KB_END_SESSION();

	Kablunk::ShutdownCore();
}

#endif
