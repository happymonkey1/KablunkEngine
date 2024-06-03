#pragma once

#include "Kablunk/networking/internal_packet_type.h"

#include <msgpack.hpp>

#include <type_traits>

namespace kb::network
{ // start namespace kb::network

struct rpc_request
{
    underlying_packet_type_t m_type{};
    u32 m_request_id{};
    std::string m_name{};
    msgpack::object m_arguments{};

    MSGPACK_DEFINE(m_type, m_request_id, m_name, m_arguments);
};

struct rpc_response
{
    underlying_packet_type_t m_type{};
    u32 m_response_id{};
    std::string m_name{};
    // buffer to actual function response
    option<msgpack::object> m_data_buffer{};

    MSGPACK_DEFINE(m_type, m_response_id, m_name, m_data_buffer);
};

} // end namespace kb::network
