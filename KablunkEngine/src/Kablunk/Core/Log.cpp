#include "kablunkpch.h"
#include "Kablunk/Core/Log.h"

#include "Kablunk/Scripts/NativeScriptEngine.h"

#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/basic_file_sink.h"

namespace Kablunk {

	void Log::Init() {
		std::vector<spdlog::sink_ptr> logSinks;
		logSinks.emplace_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
		logSinks.emplace_back(std::make_shared<spdlog::sinks::basic_file_sink_mt>("Kablunk.log", true));

		logSinks[0]->set_pattern("%^[%T] %n: %v%$");
		logSinks[1]->set_pattern("[%T] [Thread%5t] [%l] %n: %v");

		s_core_logger = new spdlog::logger{ "KABLUNK", begin(logSinks), end(logSinks) };
		s_client_logger = new spdlog::logger{ "APP", begin(logSinks), end(logSinks) };
		
		spdlog::register_logger(std::shared_ptr<spdlog::logger>(s_core_logger));
		s_core_logger->set_level(spdlog::level::trace);
		s_core_logger->flush_on(spdlog::level::trace);

		
		spdlog::register_logger(std::shared_ptr<spdlog::logger>(s_client_logger));
		s_client_logger->set_level(spdlog::level::trace);
		s_client_logger->flush_on(spdlog::level::trace);
	}

	void Log::Shutdown()
	{
		//delete s_core_logger;
		//delete s_client_logger;
	}

}

