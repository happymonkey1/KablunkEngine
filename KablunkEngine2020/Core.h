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
