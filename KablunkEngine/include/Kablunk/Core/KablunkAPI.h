#pragma once
#ifndef KABLUNK_CORE_API_H
#define KABLUNK_CORE_API_H

#ifdef KB_BUILD_DLL
#	define KB_API __declspec(dllexport)
#	define KB_API_CPP __declspec(dllexport)
#else
#	define KB_API __declspec(dllimport)
#	define KB_API_CPP __declspec(dllimport)
#endif

// Platform detection
#ifdef _WIN32
#	ifdef _WIN64
#		define KB_PLATFORM_WINDOWS
#		define BOOST_INTERPROCESS_WINDOWS
#	else
#		error "x86 is not supported!"
#	endif
#elif TARGET_OS_MAC
//defined(__APPLE__) || defined(__MACH__)
#	error "MacOS is not supported!"
#elif defined(__ANDROID__)
#	error "Android is not supported"
#elif defined(__linux__)
#	error "Linux is not supported!"
#else
#	error "Unknown platform"
#endif


#ifdef KB_DEBUG
#	ifdef KB_PLATFORM_WINDOWS
#		define KB_DEBUG_BREAK() __debugbreak()
#	else
#		error "Platform doesn't support debugbreak!"
#	endif	
#	define KB_ENABLE_ASSERTS
#	define KB_PROFILE 0
#endif

#ifdef KB_RELEASE
#	undef KB_EXCEPTION
#endif

#ifdef KB_ENABLE_ASSERTS
#	define KB_ASSERT(x, ...)      { if (!(x)) { KB_CLIENT_ERROR("Assertion Failed: {0}", __VA_ARGS__); KB_DEBUG_BREAK(); } }
#	define KB_CORE_ASSERT(x, ...) { if (!(x)) { KB_CORE_ERROR("Assertion Failed: {0}", __VA_ARGS__); KB_DEBUG_BREAK(); } } 
#else
#	define KB_ASSERT(x, ...)		{ if (!(x)) { KB_CLIENT_ERROR("Assertion Failed: {0}", __VA_ARGS__); } }
#	define KB_CORE_ASSERT(x, ...)   { if (!(x)) { KB_CORE_ERROR("Assertion Failed: {0}", __VA_ARGS__); } } 
#endif

#define DEFINE_ENUM_TYPE_STRING(T, STR) static constexpr const char* T = STR;

#endif
