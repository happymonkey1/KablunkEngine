#pragma once

#include "Kablunk/Math/vec.hpp"
#include "Kablunk/Renderer/texture_handle.h"
#include <array>

namespace kb::render
{ // start namespace kb::render

struct virtual_texture_t
{
    // virtual texture id
    virtual_texture_handle m_handle;
    // uv coordinates for the virtual texture
    std::array<vec2_packed, 4> m_uvs;
    // dimensions of the texture
    uvec2_packed m_dimensions;

    [[nodiscard]] auto get_width() const noexcept -> u32 { return m_dimensions.m_storage.m_data[0]; }
    [[nodiscard]] auto get_height() const noexcept -> u32 { return m_dimensions.m_storage.m_data[1]; }
};

static_assert(sizeof(virtual_texture_t) <= 64);

} // end namespace kb::render
