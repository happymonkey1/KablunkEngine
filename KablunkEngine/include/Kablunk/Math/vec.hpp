#pragma once

#include "Kablunk/Core/Core.h"
#include <glm/glm.hpp>

namespace kb
{ // start namespace kb
namespace concepts
{ // start namespace ::concepts
template <typename T>
concept NumericType = std::integral<T> || std::floating_point<T>;
} // end namespace ::concepts

#ifdef KB_PACKED_MATH_TYPES
// #TODO portable pack macro


namespace details
{ // start namespace ::details

template <size_t Extent, concepts::NumericType T>
struct vec_storage
{
    T m_storage[Extent];
};

template <concepts::NumericType T = f32>
struct alignas(4) vec2_packed
{
    vec_storage<2, T> m_storage;

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

template <concepts::NumericType T = f32>
struct alignas(4) vec3_packed
{
    vec_storage<3, T> m_storage;

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

template <concepts::NumericType T = f32>
struct alignas(4) vec4_packed
{
    vec_storage<4, T> m_storage;

    constexpr vec4_packed() = default;
    constexpr ~vec4_packed() = default;

    explicit constexpr vec4_packed(T p_x, T p_y, T p_z, T p_a)
        : m_storage{ p_x, p_y, p_z, p_a }
    {
    }

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

#else
using vec2_packed = glm::vec2;
using vec3_packed = glm::vec3;
#endif

} // end namespace kb
