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
#	define KB_PLATFORM_LINUX
#else
#	error "Unknown platform"
#endif

// compiler detection
#if defined(_MSC_VER) // MSVC
#   define KB_NOT_NULL _Notnull_
#   define KB_FORCE_INLINE __forceinline
#elif defined(__clang__) // CLANG
#   define KB_NOT_NULL _Nonnull
#   define KB_FORCE_INLINE [[clang::always_inline]]
#elif defined(__GNUC__) // GCC
#   define KB_NOT_NULL __attribute__((nonnull))
#   define KB_FORCE_INLINE __attribute((always_inline))
#else
#   error "Failed to detect compiler!"
#endif

// fail-safe if compiler detection couldn't define force inline
#ifndef KB_FORCE_INLINE
#   define KB_FORCE_INLINE inline
#endif

#ifdef GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#   define KB_PACKED_MATH_TYPES
#endif

#ifdef KB_DEBUG
#	if defined(KB_PLATFORM_WINDOWS)
#		define KB_DEBUG_BREAK() __debugbreak()
#	elif defined(KB_PLATFORM_LINUX)
#		define KB_DEBUG_BREAK() raise(SIGTRAP)
#	else
#		error "Platform doesn't support debugbreak!"
#	endif	
#	define KB_ENABLE_ASSERTS
#	define KB_PROFILE 0
// #	define KB_TRACK_MEMORY
//#	define KB_UNIT_TEST
#endif

#ifdef KB_RELEASE
#	undef KB_EXCEPTION
#	define KB_TRACK_MEMORY
#	undef KB_UNIT_TEST
#endif

#ifdef KB_DISTRIBUTION
#	undef KB_EXCEPTION
#	undef KB_TRACK_MEMORY
#	undef KB_ENABLE_ASSERTS
#	undef KB_UNIT_TEST
#endif

#ifdef KB_ENABLE_ASSERTS
#	define KB_ASSERT(x, ...)      { if (!(x)) { KB_CLIENT_ERROR("Assertion Failed: {0}", fmt::format(__VA_ARGS__)); KB_DEBUG_BREAK(); } }
#	define KB_CORE_ASSERT(x, ...) { if (!(x)) { KB_CORE_ERROR("Assertion Failed: {0}", fmt::format(__VA_ARGS__)); KB_DEBUG_BREAK(); } } 
#	define KB_ASSERT_NO_LOG(x)    { if (!(x)) KB_DEBUG_BREAK(); }
#else
#	define KB_ASSERT(x, ...)		{ if (!(x)) { KB_CLIENT_ERROR("Assertion Failed: {0}", __VA_ARGS__); } }
#	define KB_CORE_ASSERT(x, ...)   { if (!(x)) { KB_CORE_ERROR("Assertion Failed: {0}", __VA_ARGS__); } } 
#	define KB_ASSERT_NO_LOG(x) { }
#endif

#define DEFINE_ENUM_TYPE_STRING(T, STR) static constexpr const char* T = STR;

#endif
