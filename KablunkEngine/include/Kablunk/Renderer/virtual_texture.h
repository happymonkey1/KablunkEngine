#pragma once
#include "Kablunk/Math/vec.hpp"

#include <array>

#include "Kablunk/Renderer/texture_handle.h"

namespace kb::render
{ // start namespace kb::render

struct virtual_texture
{
    // virtual texture id
    virtual_texture_handle m_handle{};
    // uv coordinates for the virtual texture
    std::array<vec2_packed, 4> m_uvs{};
};

} // end namespace kb::render
