#pragma once

#include "Kablunk/Core/RefCounting.h"
#include "kablunk/renderer/backend/shader.h"

namespace kb::render
{ // start namespace kb::render

class shader_library : public RefCounted
{
public:
    // Engine shader names
    // Basic pipeline static shader
    inline static constexpr const char* k_diffuse_static_shader_name = "Kablunk_diffuse_static";
    // Basic pipeline animated shader
    inline static constexpr const char* k_diffuse_anim_shader_name = "Kablunk_diffuse_anim";
    // PBR pipeline static shader
    inline static constexpr const char* k_pbr_static_shader_name = "Kablunk_pbr_static";
    // Directional shadows
    inline static constexpr const char* k_directional_shadows_shader_name = "kb_directional_shadows";
    inline static constexpr const char* k_renderer_2d_quad_name = "Renderer2D_Quad";
    inline static constexpr const char* k_renderer_2d_circle_name = "Renderer2D_Circle";
    inline static constexpr const char* k_renderer_2d_line_name = "Renderer2D_Line";
    inline static constexpr const char* k_renderer_2d_text_name = "Renderer2D_Text";
    inline static constexpr const char* k_renderer_2d_UI_name = "Renderer2D_UI";
    inline static constexpr const char* k_scene_composite_name = "scene_composite";

    inline static constexpr const char* k_default_shader_name = k_diffuse_static_shader_name;
public:
    shader_library() = default;
    ~shader_library() noexcept override = default;

    auto add(const arc<backend::shader>& shader) noexcept -> const arc<backend::shader>&;
    auto add(const std::string& name, const arc<backend::shader>& shader) noexcept -> const arc<backend::shader>&;
    const arc<backend::shader>& load(const std::string& filepath);
    const arc<backend::shader>& load(const std::string& name, const std::string& filepath);

    const arc<backend::shader>& get(const std::string& name);

    bool exists(const std::string& name);

private:
    unordered_flat_map<std::string, arc<backend::shader>> m_shaders;
};

} // end namespace kb::render
