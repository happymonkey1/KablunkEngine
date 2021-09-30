#ifndef KABLUNK_CORE_CORE_H
#define KABLUNK_CORE_CORE_H

#include <memory>

#ifdef KB_BUILD_DLL
#	define KB_API __declspec(dllexport)
#else
#	define KB_API __declspec(dllimport)
#endif

// Platform detection
#ifdef _WIN32
#	ifdef _WIN64
#		define KB_PLATFORM_WINDOWS
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


#ifdef KB_ENABLE_ASSERTS
#	define KB_ASSERT(x, ...)      { if(!(x)) { KB_CLIENT_ERROR("Assertion Failed: {0}", __VA_ARGS__); KB_DEBUG_BREAK(); } }
#	define KB_CORE_ASSERT(x, ...) { if(!(x)) { KB_CORE_ERROR("Assertion Failed: {0}", __VA_ARGS__); KB_DEBUG_BREAK(); } } 
#else
#	define KB_ASSERT(x, ...)
#	define KB_CORE_ASSERT(x, ...)
#endif

#define BIT(x) (1 << x)

#define KABLUNK_BIND_EVENT_FN(x) [this](auto&&... args) -> decltype(auto) { return this->x(std::forward<decltype(args)>(args)...); }

#ifndef ENTT_ID_TYPE
#	define ENTT_ID_TYPE uint64_t
#endif

namespace Kablunk
{
	template <typename T>
	using Scope = std::unique_ptr<T>;

	template <typename T, typename ... Args>
	constexpr Scope<T> CreateScope(Args&& ... args)
	{
		return std::make_unique<T>(std::forward<Args>(args)...);
	}

	template <typename T>
	using Ref = std::shared_ptr<T>;

	template <typename T, typename ... Args>
	constexpr Ref<T> CreateRef(Args&& ... args)
	{
		return std::make_shared<T>(std::forward<Args>(args)...);
	}
}

#endif
