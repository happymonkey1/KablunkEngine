#include "kablunkpch.h"
#include "Kablunk/Core/Logger.h"

#include "Kablunk/Scripts/NativeScriptEngine.h"

#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/basic_file_sink.h"

namespace Kablunk 
{

	void Logger::init()
	{
		std::vector<spdlog::sink_ptr> log_sinks;
		log_sinks.emplace_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
		log_sinks.emplace_back(std::make_shared<spdlog::sinks::basic_file_sink_mt>("Kablunk.log", true));

		log_sinks[0]->set_pattern("%^[%T] %n: %v%$");
		log_sinks[1]->set_pattern("[%T] [Thread%5t] [%l] %n: %v");

		s_core_logger = std::make_shared<spdlog::logger>( "[Engine]", begin(log_sinks), end(log_sinks) );
		s_client_logger = std::make_shared<spdlog::logger>( "[App]", begin(log_sinks), end(log_sinks) );
		
		spdlog::register_logger(std::shared_ptr<spdlog::logger>(s_core_logger));
		s_core_logger->set_level(spdlog::level::trace);
		s_core_logger->flush_on(spdlog::level::trace);

		
		spdlog::register_logger(std::shared_ptr<spdlog::logger>(s_client_logger));
		s_client_logger->set_level(spdlog::level::trace);
		s_client_logger->flush_on(spdlog::level::trace);

		std::cout << "Log::init() called!" << std::endl;
	}

	void Logger::shutdown()
	{
		if (m_has_shutdown)
			return;

		s_core_logger.reset();
		s_client_logger.reset();

		m_has_shutdown = true;
	}

}

