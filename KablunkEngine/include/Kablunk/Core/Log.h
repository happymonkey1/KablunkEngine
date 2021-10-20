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

		static spdlog::logger* GetCoreLogger() { return s_core_logger; }
		static spdlog::logger* GetClientLogger() { return s_client_logger; }

		// #TODO DANGEROUS API
		static spdlog::logger* DLLGetCoreLoggerPtr() { return s_core_logger; }
		// #TODO DANGEROUS API
		static spdlog::logger* DLLGetClientLoggerPtr() { return s_client_logger; }

		static void Shutdown();
	private:
		inline static spdlog::logger* s_core_logger = nullptr;
		inline static spdlog::logger* s_client_logger = nullptr;
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

#ifdef KB_DEBUG
#	define KB_TIME_FUNCTION_BEGIN()	float delta##__FUNCSIG__ = PlatformAPI::GetTime();
#	define KB_TIME_FUNCTION_END(trace) \
	auto trace_msg = std::string(trace) + std::string(": {0}"); \
	KB_CORE_TRACE(trace_msg.c_str(), (PlatformAPI::GetTime() - delta##__FUNCSIG__) * 1000.0f);
#else
#	define KB_TIME_FUNCTION_BEGIN()
#	define KB_TIME_FUNCTION_END(trace)
#endif
