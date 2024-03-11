#pragma once

#include "Kablunk/Core/Core.h"

#include <glm/glm.hpp>

namespace kb
{

struct bounding_box
{
    explicit bounding_box(f32 x, f32 y, f32 p_width, f32 p_height)
        : m_pos{ x, y }, m_dimensions{ p_width, p_height }
    { }

    glm::vec2 m_pos{ 0.f };
    glm::vec2 m_dimensions{ 0.f };

    auto get_width() const noexcept -> f32 { return std::abs(m_dimensions.x); }
    auto get_height() const noexcept -> f32 { return std::abs(m_dimensions.y); }

    auto get_center() const noexcept -> glm::vec2
    {
        return { m_pos.x + (get_width() / 2.f), m_pos.y + (get_height() / 2.f) };
    }
};

}
