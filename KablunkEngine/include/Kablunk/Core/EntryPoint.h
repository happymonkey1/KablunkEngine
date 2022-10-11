#pragma once

#include "Kablunk/Core/Core.h"
#include "Kablunk/Core/Application.h"

#ifdef KB_UNIT_TEST
#	include "Kablunk/ML/Test/test_Tensor.h"
#	include "Kablunk/ML/Test/test_SimpleNN.h"
#endif

#ifdef KB_PLATFORM_WINDOWS

extern Kablunk::Application* Kablunk::CreateApplication(int argc, char** argv);

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

#ifdef KB_UNIT_TEST
	Kablunk::ml::tensor::test_tensor_main();
	Kablunk::ml::test::test_simple_nn_main();
#endif

	KB_BEGIN_SESSION("Startup", "KablunkProfile-Startup.json");
	Kablunk::Application* app = Kablunk::CreateApplication(argc, argv);
	KB_END_SESSION();


	KB_BEGIN_SESSION("Runtime", "KablunkProfile-Runtime.json");
	app->Run();
	KB_END_SESSION();

	KB_BEGIN_SESSION("Shutdown", "KablunkProfile-Shutdown.json");
	app->shutdown();
	KB_END_SESSION();
	

	Kablunk::ShutdownCore();
}

#endif
