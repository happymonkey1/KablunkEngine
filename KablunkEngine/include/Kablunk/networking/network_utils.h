#pragma once

#include <msgpack.hpp>

namespace kb::network::util
{ // start namespace kb::network::util

template <typename T>
auto convert_object(const msgpack::object& p_object) noexcept -> std::optional<T>
{
    T converted;
    try
    {
        converted = p_object.as<T>();
    }
    catch (msgpack::type_error& err)
    {
        KB_CORE_ERROR("[network_util]: Could not parse buffer as an rpc call!");
        KB_CORE_ERROR("[network_util]:  msgpack::type_error {}", err.what());
        return std::nullopt;
    }

    return converted;
}

template <typename T>
auto convert_buffer(const msgpack::sbuffer& p_buffer) noexcept -> std::optional<T>
{
    const auto handle = msgpack::unpack(
        p_buffer.data(),
        p_buffer.size()
    );

    return convert_object<T>(handle.get());
}

template <typename T>
auto as_buffer(const T& p_data) noexcept -> msgpack::sbuffer
{
    msgpack::sbuffer buffer{};
    msgpack::packer packer{ buffer };
    packer.pack(p_data);

    return buffer;
}

} // end namespace kb::network::util
