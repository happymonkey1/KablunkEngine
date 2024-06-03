#pragma once

#include "Kablunk/networking/internal_packet_type.h"

#include <msgpack.hpp>
#include <variant>

namespace kb::network
{ // start namespace kb::network

enum class internal_error_code_t : u16
{
    // invalid error code
    none = 0,
    // generic internal server error
    kb_internal_server_error,
    // unexpected (not defined) packet type was received
    kb_invalid_packet_type,
    // invalid packet data (format, expected types, etc.)
    kb_packet_validation_error,
    // client is not authenticated
    kb_not_authorized,
    // timeout duration reached
    kb_timeout,
    // reserve 0-255 for internal use
    kb_reserved = 0x00FF,
};

using underlying_error_code_type_t = std::underlying_type_t<internal_error_code_t>;

struct error_response_data
{
    // required packet type
    underlying_packet_type_t m_packet_type{};
    // required response code
    u32 m_response_id{};
    // error code
    underlying_error_code_type_t m_error_code =
        static_cast<underlying_error_code_type_t>(internal_error_code_t::kb_internal_server_error);

    MSGPACK_DEFINE(m_packet_type, m_response_id, m_error_code);
};

} // end namespace kb::network
