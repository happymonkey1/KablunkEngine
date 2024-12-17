#include "Panels/scene_renderer_panel.h"

#include "imgui.h"
#include "Kablunk/Imgui/ImGuiWrappers.h"
#include "Kablunk/Math/Math.h"
#include "Kablunk/Scene/Components.h"

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
            UI::Image(depth_image, i, { 256, 256 });
        }

        const auto& cascade_data = m_scene_renderer->m_shadow_cascade_data;
        ImGui::Text("Split Depth: %f", cascade_data.m_shadow_cascade_splits[i]);

        ImGui::Spacing();

        ++i;
    }

    ImGui::Text("Shadow Projection Translation");

    const auto& view = m_scene_renderer->m_shadow_cascade_mats.m_views[0];
    glm::vec3 translation, scale, rotation;
    kb::math::decompose_transform(view, translation, scale, rotation);
    rotation = glm::degrees(rotation);

    UI::DrawVec3Control("Position", translation);

    UI::DrawVec3Control("Rotation", rotation);

    ImGui::Spacing();

    if (ImGui::Button("Set Runtime Camera to cascade view"))
    {
        auto& scene = m_scene_renderer->m_context;
        auto view = scene->m_registry.view<CameraComponent, TransformComponent>();
        for (const auto id : view)
        {
            Entity entity{ id, scene };
            const auto& camera_comp = entity.GetComponent<CameraComponent>();
            if (!camera_comp.Primary)
                continue;

            auto& transform_comp = entity.GetComponent<TransformComponent>();
            transform_comp.Translation = translation;
            transform_comp.Rotation = glm::radians(rotation);
        }
    }

    ImGui::End();
}

} // end namespace kb
