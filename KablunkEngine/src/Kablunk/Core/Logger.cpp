#include "kablunkpch.h"
#include "Kablunk/Core/Logger.h"

#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/basic_file_sink.h"

namespace kb 
{ // start namespace kb

void Logger::init()
{
	std::vector<spdlog::sink_ptr> log_sinks;
	log_sinks.emplace_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
	log_sinks.emplace_back(std::make_shared<spdlog::sinks::basic_file_sink_mt>("Kablunk.log", true));

	log_sinks[0]->set_pattern("%^[%T] %n: %v%$");
    log_sinks[0]->set_level(spdlog::level::info);
	log_sinks[1]->set_pattern("[%T] [Thread%5t] [%l] %n: %v");
    log_sinks[1]->set_level(spdlog::level::trace);

	m_core_logger = std::make_shared<spdlog::logger>( "[Engine]", begin(log_sinks), end(log_sinks) );
	m_client_logger = std::make_shared<spdlog::logger>( "[App]", begin(log_sinks), end(log_sinks) );

	spdlog::register_logger(std::shared_ptr<spdlog::logger>(m_core_logger));
	//s_core_logger->set_level(spdlog::level::trace);
	m_core_logger->flush_on(spdlog::level::trace);

	spdlog::register_logger(std::shared_ptr<spdlog::logger>(m_client_logger));
	//s_client_logger->set_level(spdlog::level::trace);
	m_client_logger->flush_on(spdlog::level::trace);

	//std::cout << "Log::init() called!" << std::endl;
}

void Logger::shutdown()
{
	m_core_logger.reset();
	m_client_logger.reset();
}

} // end namespace kb

