#pragma once
#include "Core.h"
#include "spdlog/spdlog.h"


namespace kablunk {

	class KABLUNK_API Log {
	public:
		static void Init();

		inline static std::shared_ptr<spdlog::logger>& GetCoreLogger() { return s_CoreLogger; }
		inline static std::shared_ptr<spdlog::logger>& GetClientLogger() { return s_ClientLogger; }
	private:
		static std::shared_ptr<spdlog::logger> s_CoreLogger;
		static std::shared_ptr<spdlog::logger> s_ClientLogger;
	};

	
}


#define KB_CORE_ERROR(...) ::kablunk::Log::GetCoreLogger()->error(__VA_ARGS__)
#define KB_CORE_WARN(...) ::kablunk::Log::GetCoreLogger()->warn(__VA_ARGS__)
#define KB_CORE_INFO(...) ::kablunk::Log::GetCoreLogger()->info(__VA_ARGS__)
#define KB_CORE_TRACE(...) ::kablunk::Log::GetCoreLogger()->trace(__VA_ARGS__)
#define KB_CORE_FATAL(...) ::kablunk::Log::GetCoreLogger()->critical(__VA_ARGS__)

#define KB_CLIENT_ERROR(...) ::kablunk::Log::GetClientLogger()->error(__VA_ARGS__)
#define KB_CLIENT_WARN(...) ::kablunk::Log::GetClientLogger()->warn(__VA_ARGS__)
#define KB_CLIENT_INFO(...) ::kablunk::Log::GetClientLogger()->info(__VA_ARGS__)
#define KB_CLIENT_TRACE(...) ::kablunk::Log::GetClientLogger()->trace(__VA_ARGS__)
#define KB_CLIENT_FATAL(...) ::kablunk::Log::GetClientLogger()->critical(__VA_ARGS__)
