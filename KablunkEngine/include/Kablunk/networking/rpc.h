#pragma once

#include "Kablunk/networking/packet_type.h"

#include <msgpack.hpp>

#include <type_traits>

namespace kb::network
{ // start namespace kb::network

struct rpc_header
{
    // packet id
    u32 m_id{};
    // rpc function name
    std::string m_name{};

    // add serialization support for `rpc_header`
    MSGPACK_DEFINE(m_id, m_name);
};

struct rpc_request
{
    std::underlying_type_t<packet_type> m_type{};
    u32 m_id{};
    std::string m_name{};
    msgpack::object m_arguments{};

    MSGPACK_DEFINE(m_type, m_id, m_name, m_arguments);
};

struct rpc_response
{
    std::underlying_type_t<packet_type> m_type{};
    u32 m_id{};
    std::string m_name{};
    // buffer to function response
    msgpack::object m_data_buffer{};

    MSGPACK_DEFINE(m_type, m_id, m_name, m_data_buffer);
};

} // end namespace kb::network
