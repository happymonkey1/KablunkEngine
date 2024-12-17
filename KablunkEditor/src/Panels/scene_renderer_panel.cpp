#include "Panels/scene_renderer_panel.h"

#include "imgui.h"
#include "Kablunk/Imgui/ImGuiWrappers.h"

namespace kb
{ // start namespace kb

auto scene_renderer_panel::on_imgui_render() noexcept -> void
{
    if (!m_scene_renderer || !m_active)
        return;

    ImGui::Begin("Scene Renderer");

    ImGui::Text("Shadow Cascades");

    size_t i = 0;
    for (const auto& shadow_pass : m_scene_renderer->m_directional_shadow_pass)
    {
        const auto& depth_image = shadow_pass->get_target_frame_buffer()->get_depth_image();

        ImGui::Text("Cascade %u", i);

        if (m_scene_renderer->m_resources_created)
        {
            UI::Image(depth_image, i, { 64, 64 });
            ImGui::Spacing();
        }

        ++i;
    }

    ImGui::End();
}

} // end namespace kb
