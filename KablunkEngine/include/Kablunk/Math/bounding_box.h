#pragma once

#include "Kablunk/Core/Core.h"

#include <glm/glm.hpp>

namespace kb
{

struct bounding_box
{
    explicit bounding_box(f32 x, f32 y, f32 z, f32 w)
        : m_top_left{ x, y }, m_bottom_right{ z, w }
    { }

    glm::vec2 m_top_left{ 0.f };
    glm::vec2 m_bottom_right{ 0.f };

    auto get_width() const noexcept -> f32 { return std::abs(m_top_left.x - m_bottom_right.x); }
    auto get_height() const noexcept -> f32 { return std::abs(m_top_left.y - m_bottom_right.y); }

    auto get_center() const noexcept -> glm::vec2
    {
        return { m_top_left.x + get_width() / 2.f, m_top_left.y + get_height() / 2.f };
    }
};

}
