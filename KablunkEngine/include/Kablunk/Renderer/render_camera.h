#pragma once

#include <glm/glm.hpp>

namespace kb::render
{ // start namespace kb::render

struct render_camera
{
    glm::mat4 m_projection{};
    glm::mat4 m_view{};
};

} // end namespace kb::render
