#pragma once

#include "Kablunk/Core/handle.h"
#include "Kablunk/Core/crc_32_hash.h"

#include <string_view>

namespace kb
{ // start namespace kb

namespace details
{ // start namespace ::details

struct raw_texture_handle_tag{};

struct virtual_texture_handle_tag{};

} // end namespace ::details

using raw_texture_handle = handle<details::raw_texture_handle_tag>;
using virtual_texture_handle = handle<details::virtual_texture_handle_tag>;

// implementations for explicit conversions

// --- raw texture handle -------------------------------------------------------------------

// f32 conversion used in renderer shader api (sampler uses f32 as index for some reason...)
template <>
template <>
constexpr auto raw_texture_handle::as<f32>() const noexcept -> f32
{
    return static_cast<f32>(static_cast<value_t>(m_handle));
}

template <>
template <>
constexpr auto raw_texture_handle::as<u32>() const noexcept -> u32
{
    return static_cast<u32>(static_cast<value_t>(m_handle));
}

// #TODO check if this is not UB...
template <>
template <>
constexpr auto raw_texture_handle::into(std::string_view p_value) noexcept -> raw_texture_handle
{
    if (std::is_constant_evaluated())
    {
        return raw_texture_handle{ crc32<sizeof(p_value.data())>(p_value.data()) };
    }
    else
    {
        return raw_texture_handle{ static_cast<u32>(std::hash<std::string_view>{}(p_value)) };
    }
}

// ------------------------------------------------------------------------------------------


// --- virtual texture handle ---------------------------------------------------------------

// f32 conversion used in renderer shader api (sampler uses f32 as index for some reason...)
template <>
template <>
constexpr auto virtual_texture_handle::as<f32>() const noexcept -> f32
{
    return static_cast<f32>(static_cast<value_t>(m_handle));
}

template <>
template <>
constexpr auto virtual_texture_handle::as<u32>() const noexcept -> u32
{
    return static_cast<u32>(static_cast<value_t>(m_handle));
}

// #TODO check if this is not UB...
template <>
template <>
constexpr auto virtual_texture_handle::into(std::string_view p_value) noexcept -> virtual_texture_handle
{
    if (std::is_constant_evaluated())
    {
        return virtual_texture_handle{ crc32<sizeof(p_value.data())>(p_value.data()) };
    }
    else
    {
        return virtual_texture_handle{ static_cast<u32>(std::hash<std::string_view>{}(p_value)) };
    }
}

// ------------------------------------------------------------------------------------------

} // end namespace kb

namespace std
{ // start namespace std

// --- raw texture handle -------------------------------------------------------------------

template <>
struct std::hash<kb::raw_texture_handle>
{
    std::size_t operator()(const kb::raw_texture_handle& p_item_handle) const noexcept
    {
        return static_cast<std::uint64_t>(p_item_handle);
    }
};

// ------------------------------------------------------------------------------------------

// --- virtual texture handle ---------------------------------------------------------------

template <>
struct std::hash<kb::virtual_texture_handle>
{
    std::size_t operator()(const kb::virtual_texture_handle& p_item_handle) const noexcept
    {
        return static_cast<std::uint64_t>(p_item_handle);
    }
};

// ------------------------------------------------------------------------------------------

} // end namespace std
