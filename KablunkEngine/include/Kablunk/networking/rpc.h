#pragma once

#include <msgpack.hpp>

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
    rpc_header m_header{};
    msgpack::object m_arguments{};

    auto as_buffer() const noexcept -> msgpack::sbuffer
    {
        msgpack::sbuffer buffer{};
        msgpack::packer packer{ &buffer };
        packer.pack(m_header);
        packer.pack(m_arguments);

        return buffer;
    }

    MSGPACK_DEFINE(m_header, m_arguments);
};

struct rpc_response
{
    // rpc packet header
    rpc_header m_header{};
    // buffer to function response
    msgpack::sbuffer m_data_buffer{};
};

} // end namespace kb::network
