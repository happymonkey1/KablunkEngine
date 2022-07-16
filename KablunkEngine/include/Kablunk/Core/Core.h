#ifndef KABLUNK_CORE_CORE_H
#define KABLUNK_CORE_CORE_H

#include "Kablunk/Core/KablunkAPI.h"
#include "Kablunk/Core/RefCounting.h"
#include "Kablunk/Core/Logger.h"
#include "Kablunk/Core/Singleton.h"
#include <memory>

namespace Kablunk
{
	void InitCore();
	void ShutdownCore();
}

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
