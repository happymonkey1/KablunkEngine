#pragma once
#include "Kablunk/Core/Core.h"


// ignore warnings raises in external headers
#pragma warning(push, 0)
#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>
#pragma warning(pop)

namespace Kablunk {

	class Log {
	public:
		static void Init();

		static std::shared_ptr<spdlog::logger>& GetCoreLogger() { return s_CoreLogger; }
		static std::shared_ptr<spdlog::logger>& GetClientLogger() { return s_ClientLogger; }
	private:
		static std::shared_ptr<spdlog::logger> s_CoreLogger;
		static std::shared_ptr<spdlog::logger> s_ClientLogger;
	};

	
}


#define KB_CORE_TRACE(...)   ::Kablunk::Log::GetCoreLogger()->trace(__VA_ARGS__)
#define KB_CORE_INFO(...)    ::Kablunk::Log::GetCoreLogger()->info(__VA_ARGS__)
#define KB_CORE_WARN(...)    ::Kablunk::Log::GetCoreLogger()->warn(__VA_ARGS__)
#define KB_CORE_ERROR(...)   ::Kablunk::Log::GetCoreLogger()->error(__VA_ARGS__)
#define KB_CORE_FATAL(...)   ::Kablunk::Log::GetCoreLogger()->critical(__VA_ARGS__)

#define KB_CLIENT_TRACE(...) ::Kablunk::Log::GetClientLogger()->trace(__VA_ARGS__)
#define KB_CLIENT_INFO(...)  ::Kablunk::Log::GetClientLogger()->info(__VA_ARGS__)
#define KB_CLIENT_WARN(...)  ::Kablunk::Log::GetClientLogger()->warn(__VA_ARGS__)
#define KB_CLIENT_ERROR(...) ::Kablunk::Log::GetClientLogger()->error(__VA_ARGS__)
#define KB_CLIENT_FATAL(...) ::Kablunk::Log::GetClientLogger()->critical(__VA_ARGS__)
