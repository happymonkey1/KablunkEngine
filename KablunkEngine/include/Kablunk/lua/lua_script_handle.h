#pragma once

#include "Kablunk/Core/crc_32_hash.h"
#include "Kablunk/Core/handle.h"

namespace kb
{ // start namespace kb

namespace details
{ // start namespace ::details

struct lua_script_handle_tag{};

} // end namespace ::details

using lua_script_handle = handle<details::lua_script_handle_tag>;

template <>
template <>
constexpr auto lua_script_handle::as<u32>() const noexcept -> u32
{
    return static_cast<u32>(static_cast<value_t>(m_handle));
}

// #TODO check if this is not UB...
template <>
template <>
constexpr auto lua_script_handle::into(std::string_view p_value) noexcept -> lua_script_handle
{
    if (std::is_constant_evaluated())
    {
        return lua_script_handle{ crc32<sizeof(p_value.data())>(p_value.data()) };
    }
    else
    {
        return lua_script_handle{ static_cast<u32>(std::hash<std::string_view>{}(p_value)) };
    }
}

} // end namespace kb

namespace std
{ // start namespace std

template <>
struct std::hash<kb::lua_script_handle>
{
    std::size_t operator()(const kb::lua_script_handle& p_item_handle) const noexcept
    {
        return static_cast<std::size_t>(p_item_handle);
    }
};

} // end namespace std
