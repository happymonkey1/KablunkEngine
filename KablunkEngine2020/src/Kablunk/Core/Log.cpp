#include "kablunkpch.h"
#include "Kablunk/Core/Log.h"
#include "spdlog/sinks/stdout_color_sinks.h"

namespace Kablunk {
	std::shared_ptr<spdlog::logger> Log::s_CoreLogger;
	std::shared_ptr<spdlog::logger> Log::s_ClientLogger;

	void Log::Init() {
		spdlog::set_pattern("%^[%T] %n: %v%$");
		s_CoreLogger = spdlog::stdout_color_mt("Kablunk", spdlog::color_mode::automatic); //spdlog::stdout_color_mt("Kablunk");
		s_CoreLogger->set_level(spdlog::level::trace);

		s_ClientLogger = spdlog::stdout_color_mt("Client", spdlog::color_mode::automatic);
		s_ClientLogger->set_level(spdlog::level::trace);
	}
}

