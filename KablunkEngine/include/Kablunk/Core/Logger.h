#pragma once
#include "Kablunk/Core/Core.h"

#include "Kablunk/Core/Singleton.h"


// ignore warnings raises in external headers
#pragma warning(push, 0)
#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>
#pragma warning(pop)

namespace Kablunk 
{
	enum class LoggerType : uint8_t
	{
		CORE = 0,
		CLIENT
	};

	class KB_API Logger : public ISingleton {
	public:
		spdlog::logger* get_core_logger() { return s_core_logger; }
		spdlog::logger* get_client_logger() { return s_client_logger; }
		virtual void init() override;
		virtual void shutdown() override;

	private:
		SINGLETON_CONSTRUCTOR(Logger)

	private:
		spdlog::logger* s_core_logger = nullptr;
		spdlog::logger* s_client_logger = nullptr;

		friend class Singleton<Logger>;
	};
	
}

#define KB_CORE_TRACE(...)   ::Kablunk::Singleton<Kablunk::Logger>::get()->get_core_logger()->trace(__VA_ARGS__)
#define KB_CORE_INFO(...)    ::Kablunk::Singleton<Kablunk::Logger>::get()->get_core_logger()->info(__VA_ARGS__)
#define KB_CORE_WARN(...)    ::Kablunk::Singleton<Kablunk::Logger>::get()->get_core_logger()->warn(__VA_ARGS__)
#define KB_CORE_ERROR(...)   ::Kablunk::Singleton<Kablunk::Logger>::get()->get_core_logger()->error(__VA_ARGS__)
#define KB_CORE_FATAL(...)   ::Kablunk::Singleton<Kablunk::Logger>::get()->get_core_logger()->critical(__VA_ARGS__)
												  
#define KB_CLIENT_TRACE(...) ::Kablunk::Singleton<Kablunk::Logger>::get()->get_client_logger()->trace(__VA_ARGS__)
#define KB_CLIENT_INFO(...)  ::Kablunk::Singleton<Kablunk::Logger>::get()->get_client_logger()->info(__VA_ARGS__)
#define KB_CLIENT_WARN(...)  ::Kablunk::Singleton<Kablunk::Logger>::get()->get_client_logger()->warn(__VA_ARGS__)
#define KB_CLIENT_ERROR(...) ::Kablunk::Singleton<Kablunk::Logger>::get()->get_client_logger()->error(__VA_ARGS__)
#define KB_CLIENT_FATAL(...) ::Kablunk::Singleton<Kablunk::Logger>::get()->get_client_logger()->critical(__VA_ARGS__)

#ifdef KB_DEBUG
#	define KB_TIME_FUNCTION_BEGIN()	float delta##__FUNCSIG__ = PlatformAPI::GetTime();
#	define KB_TIME_FUNCTION_END(trace) \
	auto trace_msg = std::string(trace) + std::string(": {0}"); \
	KB_CORE_TRACE(trace_msg.c_str(), (PlatformAPI::GetTime() - delta##__FUNCSIG__) * 1000.0f);
#else
#	define KB_TIME_FUNCTION_BEGIN()
#	define KB_TIME_FUNCTION_END(trace)
#endif
