#pragma once


#ifdef KABLUNK_PLATFORM_WINDOWS
	#ifdef KABLUNK_BUILD_DLL
		#define KABLUNK_API __declspec(dllexport)
	#else
		#define KABLUNK_API __declspec(dllimport)
	#endif
#else
	#error Kablunk Engine only supports Windows! :)
#endif

#define BIT(x) (1 << x)

#define KABLUNK_BIND_EVENT_FN(x) std::bind(&x, this, std::placeholders::_1)