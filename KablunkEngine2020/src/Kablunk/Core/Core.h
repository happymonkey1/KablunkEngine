#pragma once


#ifdef KB_PLATFORM_WINDOWS
	#if KB_DYNAMIC_LINK
		#ifdef KB_BUILD_DLL
			#define KABLUNK_API __declspec(dllexport)
		#else
			#define KABLUNK_API __declspec(dllimport)
		#endif
	#else
		#define KABLUNK_API
	#endif
#else
	#error Kablunk Engine only supports Windows! :)
#endif

#ifdef KB_DEBUG
	#define KB_ENABLE_ASSERTS
#endif

#ifdef KB_ENABLE_ASSERTS

	#define KB_ASSERT(x, ...) {      if(!(x)) { KB_CLIENT_ERROR("Assertion Failed: {0}", __VA_ARGS__); __debugbreak(); } }
	#define KB_CORE_ASSERT(x, ...) { if(!(x)) { KB_CORE_ERROR("Assertion Failed: {0}", __VA_ARGS__); __debugbreak(); } } 
#else
	#define KB_ASSERT(x, ...)
	#define KB_CORE_ASSERT(x, ...)
#endif

#define BIT(x) (1 << x)

#define KABLUNK_BIND_EVENT_FN(x) std::bind(&x, this, std::placeholders::_1)
