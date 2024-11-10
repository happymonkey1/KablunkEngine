#pragma once

#include "Kablunk/Core/CoreTypes.h"
#include "Kablunk/Core/Singleton.h"

#include <format>
#include <string>
#include <utility>

// ICE in MSVC 17.7 in fmt/ostream.h, can remove when msvc updates
#define SPDLOG_FMT_EXTERNAL
// ignore warnings raises in external headers
#pragma warning(push, 0)
#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>
#pragma warning(pop)

namespace kb
{ // start namespace kb

enum class logger_type_t : uint8_t
{
	core = 0,
	client
};

class Logger {
public:
	Logger() = default;

	~Logger() noexcept { shutdown(); }

	std::shared_ptr<spdlog::logger> get_core_logger() { return m_core_logger; }
	std::shared_ptr<spdlog::logger> get_client_logger() { return m_client_logger; }

	void init();
	void shutdown();

	SINGLETON_GET_FUNC(Logger)

private:
	std::shared_ptr<spdlog::logger> m_core_logger = nullptr;
	std::shared_ptr<spdlog::logger> m_client_logger = nullptr;
};
} // end namespace kb

// #NOTE these are old variadic macros defined for logging
//       use should be phased out for templated functions defined below...

#define KB_CORE_TRACE(...)   ::kb::Singleton<kb::Logger>::get().get_core_logger()->trace(__VA_ARGS__)
#define KB_CORE_INFO(...)    ::kb::Singleton<kb::Logger>::get().get_core_logger()->info(__VA_ARGS__)
#define KB_CORE_WARN(...)    ::kb::Singleton<kb::Logger>::get().get_core_logger()->warn(__VA_ARGS__)
#define KB_CORE_ERROR(...)   ::kb::Singleton<kb::Logger>::get().get_core_logger()->error(__VA_ARGS__)
#define KB_CORE_FATAL(...)   ::kb::Singleton<kb::Logger>::get().get_core_logger()->critical(__VA_ARGS__)

#define KB_CLIENT_TRACE(...) ::kb::Singleton<kb::Logger>::get().get_client_logger()->trace(__VA_ARGS__)
#define KB_CLIENT_INFO(...)  ::kb::Singleton<kb::Logger>::get().get_client_logger()->info(__VA_ARGS__)
#define KB_CLIENT_WARN(...)  ::kb::Singleton<kb::Logger>::get().get_client_logger()->warn(__VA_ARGS__)
#define KB_CLIENT_ERROR(...) ::kb::Singleton<kb::Logger>::get().get_client_logger()->error(__VA_ARGS__)
#define KB_CLIENT_FATAL(...) ::kb::Singleton<kb::Logger>::get().get_client_logger()->critical(__VA_ARGS__)

// formatter for std::filesystem::path
// #TODO this is not efficient...
template <>
struct fmt::formatter<std::filesystem::path> : formatter<std::string_view>
{
    template <typename FormatContext>
    auto format(const std::filesystem::path& path, FormatContext& ctx)
    {
        return formatter<std::string_view>::format(path.string(), ctx);
    }
};

namespace kb::log
{  // start namespace kb::log


// enum for logger type
enum class logger_type_t : u8
{
	core,
	client
};

// enum for logger "tags", which are prepended to the log message
// tags are structured as '[TAG]'
enum class logger_tag_t
{
    // no logger tag ('')
    empty,

	// assets
	asset,
	asset_manager,

	// core
	core,
	application,
	window,

	// plugin
	plugin,
	plugin_manager,

	// project
	project,
	project_manager,
	project_serializer,

	// renderer
	renderer,
	camera,
	editor_camera,
	framebuffer,
	graphics_context,
	image,
	material,
	material_asset,
	mesh,
	pipeline,
	render_command,
	render_command_2d,
	render_command_buffer,
	render_command_queue,
	scene_renderer,
	shader,
	texture,
	vertex_buffer,
	index_buffer,

	// scene
	scene,
	entity,
	component,
	scene_camera,
	scene_serializer,

	// scripting
	csharp_script_engine,
	native_script_general,
	native_script,
	native_script_engine,
	native_script_module,

    // network
    network,
    network_server,
    network_client,

	// utility
	utility,

	NONE
};

using logger_tags = logger_tag_t;

// helper function to convert enum to cstr
// #TODO look into boost preprocessor or roll a custom solution so a switch
//		 statement is not necessary...
inline constexpr auto logger_tag_to_cstr(logger_tag_t logger_tag) noexcept -> std::string_view
{
	// huge switch statement of compiled, core tags
	// when adding a new tag to the logger_tag_t enum,
	// add corresponding cstring return here...
	switch (logger_tag)
	{
    case logger_tag_t::empty:                       return "";
	case logger_tag_t::asset:					return "asset";
	case logger_tag_t::asset_manager:			return "asset_manager";
	case logger_tag_t::core:					return "core";
	case logger_tag_t::application:				return "application";
	case logger_tag_t::window:					return "window";
	case logger_tag_t::plugin:					return "plugin";
	case logger_tag_t::plugin_manager:			return "plugin_manager";
	case logger_tag_t::project:					return "project";
	case logger_tag_t::project_manager:			return "project_manager";
	case logger_tag_t::project_serializer:		return "project_serializer";
	case logger_tag_t::renderer:				return "renderer";
	case logger_tag_t::camera:					return "camera";
	case logger_tag_t::editor_camera:			return "editor";
	case logger_tag_t::framebuffer:				return "framebuffer";
	case logger_tag_t::graphics_context:		return "graphics_context";
	case logger_tag_t::image:					return "image";
	case logger_tag_t::material:				return "material";
	case logger_tag_t::material_asset:			return "material_asset";
	case logger_tag_t::mesh:					return "mesh";
	case logger_tag_t::pipeline:				return "pipeline";
	case logger_tag_t::render_command:			return "render_command";
	case logger_tag_t::render_command_2d:		return "render_command_2d";
	case logger_tag_t::render_command_buffer:	return "render_command_buffer";
	case logger_tag_t::render_command_queue:	return "render_command_queue";
	case logger_tag_t::scene_renderer:			return "scene_renderer";
	case logger_tag_t::shader:					return "shader";
	case logger_tag_t::texture:					return "texture";
	case logger_tag_t::vertex_buffer:			return "vertex_buffer";
	case logger_tag_t::index_buffer:			return "index_buffer";
	case logger_tag_t::scene:					return "scene";
	case logger_tag_t::entity:					return "entity";
	case logger_tag_t::component:				return "component";
	case logger_tag_t::scene_camera:			return "scene_camera";
	case logger_tag_t::scene_serializer:		return "scene_serializer";
	case logger_tag_t::csharp_script_engine:	return "csharp_script_engine";
	case logger_tag_t::native_script_general:	return "native_script_general";
	case logger_tag_t::native_script:			return "native_script";
	case logger_tag_t::native_script_engine:	return "native_script_engine";
	case logger_tag_t::native_script_module:	return "native_script_module";
    case logger_tag_t::network:                 return "network";
    case logger_tag_t::network_server:          return "network_server";
    case logger_tag_t::network_client:          return "network_client";
	case logger_tag_t::utility:					return "utility";
	case logger_tag_t::NONE:					return "INVALID_TAG";
    default:
    {
        assert(false, "unhandled logger_tag_t!");
        return "UNHANDLED_TAG";
    }
	}
}

// enum class for the logger display level
enum class logger_level_t
{
	trace,
	info,
	warn,
	error,
	critical,

	NONE
};

namespace details
{ // start namespace ::details

// internal message formatting func for Error C7595
template <typename T1, typename... args_t>
std::string create_log_message(T1&& p_format, args_t&&... args)
{
	return std::vformat(p_format, std::make_format_args(std::forward<args_t>(args)...));
}

} // end namespace ::details

// generic logging helper to log a message (potentially with a tag)
// logger_type specifies which "class" of logger to use (core or client)
// level specifies what level of message to log (trace, info, warn, etc.)
// log_tag is an optional tag to prepend to the message
// the log tag is formatted as '[TAG]'
auto log_message(
    logger_type_t logger_type,
    logger_level_t level,
    std::string_view log_tag,
    std::string_view p_format_msg,
    auto&&... args
) noexcept -> void
{
	// get the correct logger
	auto logger = logger_type == logger_type_t::core ?
        Singleton<Logger>::get().get_core_logger() :
        Singleton<Logger>::get().get_client_logger();
	// create log formatting string
	constexpr std::string_view log_message = "[{}] {}";
#if 0
	// #TODO see if we can get compile time formatting working...
	// #TODO there is probably a more elegant solution without the use of fmt::format before sending to logger...
	std::string formatted_msg = std::vformat(
        log_message,
        std::make_format_args(log_tag, details::create_log_message(std::forward<args_t>(args)...))
    );
#endif
    std::string formatted_message;

    if constexpr (sizeof...(args) == 0)
    {
        formatted_message = fmt::format(
            log_message,
            std::remove_cvref_t<decltype(log_tag)>(log_tag),
            static_cast<std::remove_cvref_t<decltype(p_format_msg)>>(p_format_msg)
        );
    }
    else
    {
        formatted_message = fmt::format(
            log_message,
            log_tag,
            // #TODO: can we use compile time formatting here...
            fmt::vformat(
                p_format_msg,
                fmt::make_format_args(args...)
            )
        );
    }

	switch (level)
	{
		case logger_level_t::trace:		logger->trace(formatted_message); break;
		case logger_level_t::info:		logger->info(formatted_message); break;
		case logger_level_t::warn:		logger->warn(formatted_message); break;
		case logger_level_t::error:		logger->error(formatted_message); break;
		case logger_level_t::critical:	logger->critical(formatted_message); break;
        case logger_level_t::NONE:      assert(false, "Invalid logger level!"); break;
		default:
            assert(false, "Undefined logger level!");
	}
}

// wrapper to easily create a trace level message with a compiled logger tag
template <typename... args_t>
void log_message_trace(logger_type_t logger_type, logger_tag_t logger_tag, args_t&&... args)
{
	log_message(logger_type, logger_level_t::trace, logger_tag_to_cstr(logger_tag), std::forward<args_t>(args)...);
}

// wrapper to easily create an info level message with a compiled logger tag
template <typename... args_t>
void log_message_info(logger_type_t logger_type, logger_tag_t logger_tag, args_t&&... args)
{
	log_message(logger_type, logger_level_t::info, logger_tag_to_cstr(logger_tag), std::forward<args_t>(args)...);
}

// wrapper to easily create an warning level message with a compiled logger tag
template <typename... args_t>
void log_message_warn(logger_type_t logger_type, logger_tag_t logger_tag, args_t&&... args)
{
	log_message(logger_type, logger_level_t::warn, logger_tag_to_cstr(logger_tag), std::forward<args_t>(args)...);
}

// wrapper to easily create an error level message with a compiled logger tag
template <typename... args_t>
void log_message_error(logger_type_t logger_type, logger_tag_t logger_tag, args_t&&... args)
{
	log_message(logger_type, logger_level_t::error, logger_tag_to_cstr(logger_tag), std::forward<args_t>(args)...);
}

// wrapper to easily create an critical level message with a compiled logger tag
template <typename... ArgsT>
void log_message_critical(
    logger_type_t logger_type,
    const logger_tag_t logger_tag,
    ArgsT&&... args
)
{
	log_message(logger_type, logger_level_t::critical, logger_tag_to_cstr(logger_tag), std::forward<ArgsT>(args)...);
}

// helper functions for logging as client
namespace client
{ // start namespace ::client

template <typename... ArgsT>
auto trace(const logger_tag_t p_logger_tag, ArgsT&&... p_args) noexcept -> void
{
    log_message(
        logger_type_t::client,
        logger_level_t::trace,
        logger_tag_to_cstr(p_logger_tag),
        std::forward<ArgsT>(p_args)...
    );
}

template <typename... ArgsT>
auto info(const logger_tag_t p_logger_tag, ArgsT&&... p_args) noexcept -> void
{
    log_message(
        logger_type_t::client,
        logger_level_t::info,
        logger_tag_to_cstr(p_logger_tag),
        std::forward<ArgsT>(p_args)...
    );
}

template <typename... ArgsT>
auto warn(const logger_tag_t p_logger_tag, ArgsT&&... p_args) noexcept -> void
{
    log_message(
        logger_type_t::client,
        logger_level_t::warn,
        logger_tag_to_cstr(p_logger_tag),
        std::forward<ArgsT>(p_args)...
    );
}

template <typename... ArgsT>
auto error(const logger_tag_t p_logger_tag, ArgsT&&... p_args) noexcept -> void
{
    log_message(
        logger_type_t::client,
        logger_level_t::error,
        logger_tag_to_cstr(p_logger_tag),
        std::forward<ArgsT>(p_args)...
    );
}

template <typename... ArgsT>
auto critical(const logger_tag_t p_logger_tag, ArgsT&&... p_args) noexcept -> void
{
    log_message(
        logger_type_t::client,
        logger_level_t::critical,
        logger_tag_to_cstr(p_logger_tag),
        std::forward<ArgsT>(p_args)...
    );
}

} // end namespace ::client

// helper function for logging as core
namespace core
{ // start namespace ::core

auto trace(const logger_tag_t p_logger_tag, const char* p_format_msg, auto&&... p_args) noexcept -> void
{
    log_message(
        logger_type_t::core,
        logger_level_t::trace,
        logger_tag_to_cstr(p_logger_tag),
        p_format_msg,
        std::forward<decltype(p_args)>(p_args)...
    );
}

auto info(const logger_tag_t p_logger_tag, const char* p_format_msg, auto&&... p_args) noexcept -> void
{
    log_message(
        logger_type_t::core,
        logger_level_t::info,
        logger_tag_to_cstr(p_logger_tag),
        p_format_msg,
        std::forward<decltype(p_args)>(p_args)...
    );
}

auto warn(const logger_tag_t p_logger_tag, const char* p_format_msg, auto&&... p_args) noexcept -> void
{
    log_message(
        logger_type_t::core,
        logger_level_t::warn,
        logger_tag_to_cstr(p_logger_tag),
        p_format_msg,
        std::forward<decltype(p_args)>(p_args)...
    );
}

auto error(const logger_tag_t p_logger_tag, const char* p_format_msg, auto&&... p_args) noexcept -> void
{
    log_message(
        logger_type_t::core,
        logger_level_t::error,
        logger_tag_to_cstr(p_logger_tag),
        p_format_msg,
        std::forward<decltype(p_args)>(p_args)...
    );
}

auto critical(const logger_tag_t p_logger_tag, const char* p_format_msg, auto&&... p_args) noexcept -> void
{
    log_message(
        logger_type_t::core,
        logger_level_t::critical,
        logger_tag_to_cstr(p_logger_tag),
        p_format_msg,
        std::forward<decltype(p_args)>(p_args)...
    );
}

} // end namespace ::core


}  // end namespace kb::log

#ifdef KB_DEBUG
#	define KB_TIME_FUNCTION_BEGIN()	float delta##__FUNCSIG__ = PlatformAPI::GetTime();
#	define KB_TIME_FUNCTION_END(trace) \
	auto trace_msg = std::string(trace) + std::string(": {0}"); \
	KB_CORE_TRACE(trace_msg.c_str(), (PlatformAPI::GetTime() - delta##__FUNCSIG__) * 1000.0f);
#else
#	define KB_TIME_FUNCTION_BEGIN()
#	define KB_TIME_FUNCTION_END(trace)
#endif
