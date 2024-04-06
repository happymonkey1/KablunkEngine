#pragma once

#include "Kablunk/Core/handle.h"
#include "Kablunk/Core/crc_32_hash.h"

#include <string_view>

namespace kb
{ // start namespace kb

namespace details
{ // start namespace ::details

struct texture_handle_tag{};

} // end namespace ::details

using texture_handle = handle<details::texture_handle_tag>;

// implementations for explicit conversions

// f32 conversion used in renderer shader api (sampler uses f32 as index for some reason...)
template <>
template <>
constexpr auto texture_handle::as<f32>() const noexcept -> f32
{
    return static_cast<f32>(static_cast<value_t>(m_handle));
}

// #FIXME clang-diagnostic-invalid-constexpr
template <>
template <>
constexpr auto texture_handle::into(std::string_view p_value) noexcept -> texture_handle
{
    if (std::is_constant_evaluated())
    {
        return texture_handle{ crc32<sizeof(p_value.data())>(p_value.data()) };
    }
    else
    {
        return texture_handle{ static_cast<u32>(std::hash<std::string_view>{}(p_value)) };
    }
}

} // end namespace kb

namespace std
{ // start namespace std

template <>
struct std::hash<kb::texture_handle>
{
    std::size_t operator()(const kb::texture_handle& p_item_handle) const noexcept
    {
        return static_cast<std::uint64_t>(p_item_handle);
    }
};

} // end namespace std
