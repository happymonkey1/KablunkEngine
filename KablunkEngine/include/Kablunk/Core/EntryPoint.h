#pragma once

#include "Kablunk/Core/Core.h"
#include "Kablunk/Core/Application.h"

#ifdef KB_UNIT_TEST
#	include "Kablunk/ML/Test/test_Tensor.h"
#	include "Kablunk/ML/Test/test_SimpleNN.h"
#	include "Kablunk/Utilities/Containers/Test/test_FlatHashMap.h"
#endif

#ifdef KB_PLATFORM_WINDOWS

extern Kablunk::Application* Kablunk::CreateApplication(int argc, char** argv);

// auto select high performance gpu
#ifdef KB_PLATFORM_WINDOWS
#	ifndef PREFER_HIGH_PERFORMANCE_GPU
#	define PREFER_HIGH_PERFORMANCE_GPU
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
	// test tensor
	Kablunk::ml::tensor::test_tensor_main();
	// test simple nn 
	Kablunk::ml::test::test_simple_nn_main();
	// test flat hash map
	Kablunk::util::container::test::test_flat_hash_map_main();
#endif

	Kablunk::Application* app = Kablunk::CreateApplication(argc, argv);

	app->Run();

	app->shutdown();
	

	Kablunk::ShutdownCore();
}

#endif
