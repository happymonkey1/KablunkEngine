#include "kablunkpch.h"

#include "kablunk/renderer/backend/vulkan/vulkan_render_pass.h"

#include "kablunk/renderer/backend/vulkan/vulkan_context.h"
#include "Kablunk/Renderer/RenderCommand.h"

namespace kb::render::backend::vk
{ // start namespace kb::render::backend::vk

vulkan_render_pass::vulkan_render_pass(const render_pass_specification& p_specification)
	: m_specification{ p_specification }
{
    KB_CORE_ASSERT(
        p_specification.m_pipeline,
        "[vulkan_render_pass]: Pipeline can not be null!"
    );

    const descriptor_set_manager_specification descriptor_set_manager_specification{
        .m_shader = p_specification.m_pipeline->get_specification().shader.As<vulkan_shader>(),
        .m_debug_name = p_specification.m_debug_name,
        .m_start_set = 1,
        .m_end_set = 3,
        .m_default_resources = true
    };
    m_descriptor_set_manager = vulkan_descriptor_set_manager{ descriptor_set_manager_specification };
}

void vulkan_render_pass::set_input(std::string_view p_name, arc<uniform_buffer> p_uniform_buffer) noexcept
{
    m_descriptor_set_manager.set_input(p_name, p_uniform_buffer);
}

void vulkan_render_pass::set_input(std::string_view p_name, arc<uniform_buffer_set> p_uniform_buffer_set) noexcept
{
    m_descriptor_set_manager.set_input(p_name, p_uniform_buffer_set);
}

void vulkan_render_pass::set_input(std::string_view p_name, arc<storage_buffer> p_storage_buffer) noexcept
{
    m_descriptor_set_manager.set_input(p_name, p_storage_buffer);
}

void vulkan_render_pass::set_input(std::string_view p_name, arc<storage_buffer_set> p_storage_buffer_set) noexcept
{
    m_descriptor_set_manager.set_input(p_name, p_storage_buffer_set);
}

void vulkan_render_pass::set_input(std::string_view p_name, arc<texture_2d> p_texture_2d) noexcept
{
    m_descriptor_set_manager.set_input(p_name, p_texture_2d);
}

void vulkan_render_pass::set_input(std::string_view p_name, arc<image_2d> p_image_2d) noexcept
{
    m_descriptor_set_manager.set_input(p_name, p_image_2d);
}

arc<image_2d> vulkan_render_pass::get_output_image(u32 p_index) noexcept
{
    const auto& frame_buffer = m_specification.m_pipeline->get_specification().m_target_frame_buffer;

    // invalid attachment index
    if (p_index > frame_buffer->get_color_attachment_count() + 1)
    {
        log::core::warn(
            log::logger_tag_t::renderer,
            "[vulkan_render_pass]: vulkan_descriptor_set_manager can not find a valid attachment index '{}'",
            p_index
        );
        return arc<image_2d>{};
    }
    if (p_index < frame_buffer->get_color_attachment_count())
        return frame_buffer->get_image(p_index);
    return frame_buffer->get_depth_image();
}

arc<image_2d> vulkan_render_pass::get_depth_output() noexcept
{
    auto frame_buffer = m_specification.m_pipeline->get_specification().m_target_frame_buffer;
    return frame_buffer->has_depth_attachment() ? frame_buffer->get_depth_image() : arc<image_2d>{};
}

u32 vulkan_render_pass::get_first_set_index() const
{
    const auto index = m_descriptor_set_manager.get_first_set_index();
    KB_CORE_ASSERT(
        index.has_value(),
        "[vulkan_render_pass]: vulkan_descriptor_set_manager failed to get first index?"
    );
    return *index;
}

arc<pipeline> vulkan_render_pass::get_pipeline() const noexcept
{
    return m_specification.m_pipeline;
}

arc<frame_buffer> vulkan_render_pass::get_target_frame_buffer() const noexcept
{
    return m_specification.m_pipeline->get_specification().m_target_frame_buffer;
}

bool vulkan_render_pass::validate() noexcept
{
    return m_descriptor_set_manager.validate();
}

void vulkan_render_pass::bake() noexcept
{
    m_descriptor_set_manager.bake();
}

bool vulkan_render_pass::is_baked() const noexcept
{
    return static_cast<bool>(m_descriptor_set_manager.get_descriptor_pool());
}

void vulkan_render_pass::rt_prepare() noexcept
{
    m_descriptor_set_manager.rt_invalidate_and_update();
}

auto vulkan_render_pass::has_descriptor_sets() const noexcept -> bool
{
    return m_descriptor_set_manager.has_descriptor_sets();
}

auto vulkan_render_pass::get_descriptor_sets(u32 p_frame_index) const noexcept -> const std::vector<VkDescriptorSet>&
{
    return m_descriptor_set_manager.get_descriptor_sets(p_frame_index);
}

auto vulkan_render_pass::is_input_valid(std::string_view p_name) const noexcept -> bool
{
    return m_descriptor_set_manager.is_input_valid(p_name);
}

auto vulkan_render_pass::get_input_declaration(
    std::string_view p_name) const noexcept -> const render_pass_input_declaration*
{
    return m_descriptor_set_manager.get_input_declaration(p_name);
}

auto vulkan_render_pass::is_invalidated(u32 p_set, u32 p_binding) const -> bool
{
    return m_descriptor_set_manager.is_invalidated(p_set, p_binding);
}

} // end namespace kb::render::backend::vk
