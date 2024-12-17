#pragma once

#include "Kablunk/renderer/scene_renderer.h"

namespace kb
{ // start namespace kb

class scene_renderer_panel
{
public:
    scene_renderer_panel() noexcept = default;

    scene_renderer_panel(
        const arc<render::scene_renderer>& p_scene_renderer
    ) noexcept : m_scene_renderer{ p_scene_renderer } { }

    ~scene_renderer_panel() noexcept = default;

    auto on_imgui_render() noexcept -> void;

    auto set_scene_renderer(const arc<render::scene_renderer>& p_scene_renderer) noexcept
    {
        m_scene_renderer = p_scene_renderer;
    }

    auto get_active() const noexcept -> bool { return m_active; }
    auto set_active(bool p_active) noexcept { m_active = p_active; }

private:
    arc<render::scene_renderer> m_scene_renderer{};
    bool m_active = false;
};

} // end namespace kb
