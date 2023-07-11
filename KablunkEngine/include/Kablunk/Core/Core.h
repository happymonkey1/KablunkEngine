#ifndef KABLUNK_CORE_CORE_H
#define KABLUNK_CORE_CORE_H

#include "Kablunk/Core/CoreTypes.h"
#include "Kablunk/Core/KablunkAPI.h"
#include "Kablunk/Core/RefCounting.h"
#include "Kablunk/Core/Logger.h"
#include "Kablunk/Core/Singleton.h"
#include "Kablunk/Core/Memory/Memory.h"
#include "Kablunk/Core/Expected.hpp"
#include "Kablunk/Debugging/profiling.h"
#include <memory>

namespace Kablunk
{
	void InitCore();
	void ShutdownCore();
}

#if defined(KB_PLATFORM_WINDOWS)
#	define KB_FORCE_INLINE __forceinline
#elif defined(KB_PLATFORM_LINUX)
#	if defined(GCC_VERSION)  // check if compiling with gcc
#		define KB_FORCE_INLINE __attribute__((always_inline))
#	endif
#else
	// #TODO(Sean) force inline on other platforms
#	define KB_FORCE_INLINE inline
#	warning "force inlined not defined for platform!"
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
}

#endif
