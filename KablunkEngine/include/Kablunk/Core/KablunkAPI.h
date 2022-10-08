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
#	define KB_TRACK_MEMORY
#	define KB_UNIT_TEST
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
#	define KB_ASSERT(x, ...)      { if (!(x)) { KB_CLIENT_ERROR("Assertion Failed: {0}", __VA_ARGS__); KB_DEBUG_BREAK(); } }
#	define KB_CORE_ASSERT(x, ...) { if (!(x)) { KB_CORE_ERROR("Assertion Failed: {0}", __VA_ARGS__); KB_DEBUG_BREAK(); } } 
#	define KB_ASSERT_NO_LOG(x)    { if (!(x)) KB_DEBUG_BREAK(); }
#else
#	define KB_ASSERT(x, ...)		{ if (!(x)) { KB_CLIENT_ERROR("Assertion Failed: {0}", __VA_ARGS__); } }
#	define KB_CORE_ASSERT(x, ...)   { if (!(x)) { KB_CORE_ERROR("Assertion Failed: {0}", __VA_ARGS__); } } 
#	define KB_ASSERT_NO_LOG(x) { }
#endif

// #TODO fix macros
#if 0
#ifdef KB_UNIT_TEST
#	define KB_BEGIN_UNIT_TEST_GROUP(x)	\
	{ \
		int tests_passed = 0; \
		int total_tests = 0; \
		const char* test_name = x; \
		std::cout << "[Test] Starting tests for " << test_name << std::endl;;

#	define KB_UNIT_TEST(x) \
		{ \ 
total_tests++; \
if (!(x)) \
{ \
std::cout << "[Test #" << total_tests "] Failed!" << total_tests << std::endl; \
} \
else \
{ \
std::cout << "[Test #" << total_tests "] Passed!" << std::endl; \
tests_passed++; \
}
		}
#	define KB_END_UNIT_TEST_GROUP()	\
		std::cout << "[Test] Finished tests for " test_name); \
		std::cout << "[Test]  " << tests_passed << "/" << total_tests << " tests passed!" << std::endl; \
	}
#else
#	define KB_BEGIN_UNIT_TEST(x) {}
#	define KB_UNIT_TEST(x)       {}
#	define KB_END_UNIT_TEST()    {}
#endif
#endif

#define DEFINE_ENUM_TYPE_STRING(T, STR) static constexpr const char* T = STR;

#endif
