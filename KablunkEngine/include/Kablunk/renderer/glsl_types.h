#pragma once

#include "Kablunk/Core/CoreTypes.h"
#include "Kablunk/Math/vec.hpp"

namespace kb::render::glsl
{ // start namespace kb::render::glsl

// Trivial glsl boolean class which wraps a u32 (glsl booleans are 32 bits)
struct boolean
{
    u32 m_value;

    constexpr boolean() noexcept = default;

    constexpr boolean(bool p_value) : m_value{ static_cast<u32>(p_value) } { }
    auto operator=(bool p_value) noexcept -> boolean& { m_value = static_cast<u32>(p_value); return *this; }

    // Copy constructor
    constexpr boolean(const boolean&) noexcept = default;
    constexpr auto operator=(const boolean&) noexcept -> boolean& = default;
    // Move constructor
    constexpr boolean(boolean&&) noexcept = default;
    constexpr auto operator=(boolean&&) noexcept -> boolean& = default;

    constexpr ~boolean() noexcept = default;

    constexpr auto operator<=>(const boolean&) const noexcept = default;
    constexpr operator bool() const noexcept { return m_value; }
};

using vec2 = vec2_packed;
using vec3 = vec3_packed;
using vec4 = vec4_packed;

} // end namespace kb::render::glsl
