#pragma once

#include "Kablunk/Core/Core.h"
#include "Kablunk/Core/concepts.hpp"

#include <glm/glm.hpp>

#include <xmmintrin.h>

namespace kb
{ // start namespace kb

#ifdef KB_PACKED_MATH_TYPES
// #TODO portable pack macro

namespace details
{ // start namespace ::details

template <size_t Extent, concepts::NumericT T>
requires(sizeof(T) == 4)
struct vec_storage
{
    T m_data[Extent];
};

template <concepts::NumericT T = f32>
requires(sizeof(T) == 4)
struct alignas(4) vec2_packed
{
    union
    {
        // Handy accessors
        struct
        {
            T x;
            T y;
        };

        // Memory layout
        vec_storage<2, T> m_storage;
    };

    constexpr vec2_packed() = default;
    constexpr ~vec2_packed() = default;

    explicit constexpr vec2_packed(T p_x, T p_y)
        : m_storage{ p_x, p_y }
    { }

    constexpr vec2_packed(const glm::vec2& p_vec)
        : m_storage{ p_vec.x, p_vec.y }
    {
    }

    constexpr vec2_packed& operator=(const glm::vec2& p_vec) noexcept
    {
        m_storage = { p_vec.x, p_vec.y };
        return *this;
    }
};

template <concepts::NumericT T = f32>
requires(sizeof(T) == 4)
struct alignas(4) vec3_packed
{
    union
    {
        // Handy accessors
        struct
        {
            T x;
            T y;
            T z;
        };

        // Memory layout
        vec_storage<3, T> m_storage;
    };

    constexpr vec3_packed() = default;
    constexpr ~vec3_packed() = default;

    explicit constexpr vec3_packed(T p_x, T p_y, T p_z)
        : m_storage{ p_x, p_y, p_z }
    {
    }

    constexpr vec3_packed(const glm::vec3& p_vec)
        : m_storage{ p_vec.x, p_vec.y, p_vec.z }
    {
    }

    constexpr vec3_packed& operator=(const glm::vec3& p_vec) noexcept
    {
        m_storage = { p_vec.x, p_vec.y, p_vec.z };
        return *this;
    }
};

template <concepts::NumericT T = f32>
requires(sizeof(T) == 4)
struct alignas(4) vec4_packed
{
    union
    {
        // Handy accessors
        struct
        {
            T x;
            T y;
            T z;
            T w;
        };

        // Memory layout
        vec_storage<4, T> m_storage;
    };

    constexpr vec4_packed() = default;
    constexpr ~vec4_packed() = default;

    explicit constexpr vec4_packed(T p_x, T p_y, T p_z, T p_a)
        : m_storage{ p_x, p_y, p_z, p_a }
    { }

    explicit constexpr vec4_packed(const vec3_packed<T>& p_other)
        : x{ p_other.x }, y{ p_other.y }, z{ p_other.z }, w{ p_other.w }
    { }

    constexpr vec4_packed(const glm::vec4& p_vec)
        : m_storage{ p_vec.x, p_vec.y, p_vec.z, p_vec.a }
    {
    }

    constexpr vec4_packed& operator=(const glm::vec4& p_vec) noexcept
    {
        m_storage = { p_vec.x, p_vec.y, p_vec.z, p_vec.a };
        return *this;
    }
};
} // end namespace ::details

// type aliases
using vec2_packed = details::vec2_packed<>;
using vec3_packed = details::vec3_packed<>;
using vec4_packed = details::vec4_packed<>;

static_assert(sizeof(vec2_packed) == 8ull);
static_assert(sizeof(vec3_packed) == 12ull);
static_assert(sizeof(vec4_packed) == 16ull);

using uvec2_packed = details::vec2_packed<u32>;
using uvec3_packed = details::vec3_packed<u32>;
using uvec4_packed = details::vec4_packed<u32>;

static_assert(sizeof(uvec2_packed) == 8ull);
static_assert(sizeof(uvec3_packed) == 12ull);
static_assert(sizeof(uvec4_packed) == 16ull);

// utility function to translate from kb vec to glm vec
inline auto vec3_packed_to_glm_vec3(const vec3_packed& p_vec) noexcept -> glm::vec3
{
    return {
        p_vec.x,
        p_vec.y,
        p_vec.z
    };
}

#else
using vec2_packed = glm::vec2;
using vec3_packed = glm::vec3;
using vec4_packed = glm::vec4;

using uvec2_packed = glm::uvec2;
using uvec3_packed = glm::uvec3;
using uvec4_packed = glm::uvec4;
#endif

} // end namespace kb
