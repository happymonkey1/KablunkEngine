#pragma once

#include "Kablunk/Core/crc_32_hash.h"
#include "Kablunk/Core/handle.h"

namespace kb
{ // start namespace kb

namespace details
{ // start namespace ::details

struct mesh_handle_tag{};

} // end namespace ::details

using mesh_handle = handle<details::mesh_handle_tag>;

// f32 conversion used in renderer shader api (sampler uses f32 as index for some reason...)
template <>
template <>
constexpr auto mesh_handle::as<f32>() const noexcept -> f32
{
    return static_cast<f32>(static_cast<value_t>(m_handle));
}

template <>
template <>
constexpr auto mesh_handle::as<u32>() const noexcept -> u32
{
    return static_cast<u32>(static_cast<value_t>(m_handle));
}

// #TODO check if this is not UB...
template <>
template <>
constexpr auto mesh_handle::into(std::string_view p_value) noexcept -> mesh_handle
{
    if (std::is_constant_evaluated())
    {
        return mesh_handle{ crc32<sizeof(p_value.data())>(p_value.data()) };
    }
    else
    {
        return mesh_handle{ static_cast<u32>(std::hash<std::string_view>{}(p_value)) };
    }
}

} // end namespace kb

namespace std
{ // start namespace std

template <>
struct std::hash<kb::mesh_handle>
{
    std::size_t operator()(const kb::mesh_handle& p_item_handle) const noexcept
    {
        return static_cast<std::size_t>(p_item_handle);
    }
};

}
