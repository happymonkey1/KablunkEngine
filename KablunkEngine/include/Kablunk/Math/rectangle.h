#pragma once

#include "Kablunk/Core/CoreTypes.h"

#include <cmath>

namespace kb::math
{ // start namespace kb

// #TODO concept for integral type
template <typename T>
struct rectangle
{
    T m_left = 0ul;
    T m_top = 0ul;
    T m_right = 0ul;
    T m_bottom = 0ul;

    constexpr auto get_width() const -> T { return std::abs(m_right - m_left); }
    constexpr auto get_height() const -> T { return std::abs(m_bottom - m_top); }
};

} // end namespace kb


// type aliases
namespace kb
{ // start namespace kb

using rect_i32 = math::rectangle<i32>;
using rect_u32 = math::rectangle<u32>;

} // end namespace kb
