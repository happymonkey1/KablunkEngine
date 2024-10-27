#pragma once
#ifndef KABLUNK_RENDERER_BACKEND_VULKAN_RENDER_PASS_H
#define KABLUNK_RENDERER_BACKEND_VULKAN_RENDER_PASS_H

#include "Kablunk/renderer/backend/render_pass.h"

#include <vulkan/vulkan.h>

#include "kablunk/renderer/backend/vulkan/vulkan_descriptor_set_manager.h"

namespace kb::render::backend::vk
{ // start namespace kb::backend::vk

class vulkan_render_pass final : public render_pass
{
public:
	vulkan_render_pass(const render_pass_specification& p_specification);
	~vulkan_render_pass() noexcept override = default;

	render_pass_specification& get_specification() override { return m_specification; }
	const render_pass_specification& get_specification() const override { return m_specification; }

    // #TODO concept template like vulkan_descriptor_set_manager...
    void set_input(std::string_view p_name, arc<uniform_buffer> p_uniform_buffer) noexcept override;
    void set_input(std::string_view p_name, arc<uniform_buffer_set> p_uniform_buffer_set) noexcept override;
    void set_input(std::string_view p_name, arc<storage_buffer> p_storage_buffer) noexcept override;
    void set_input(std::string_view p_name, arc<storage_buffer_set> p_storage_buffer_set) noexcept override;
    void set_input(std::string_view p_name, arc<texture_2d> p_texture_2d) noexcept override;
    void set_input(std::string_view p_name, arc<image_2d> p_image_2d) noexcept override;

    // Returns either a valid color attachment, depth attachment, or null
    arc<image_2d> get_output_image(u32 p_index) noexcept override;
    arc<image_2d> get_depth_output() noexcept override;
    u32 get_first_set_index() const override;
    arc<pipeline> get_pipeline() const noexcept override;
    arc<frame_buffer> get_target_frame_buffer() const noexcept override;

    [[nodiscard]] bool validate() noexcept override;
    void bake() noexcept override;
    [[nodiscard]] bool is_baked() const noexcept override;
    void rt_prepare() noexcept override;

    [[nodiscard]] auto has_descriptor_sets() const noexcept -> bool;
    [[nodiscard]] auto get_descriptor_sets(u32 p_frame_index) const noexcept -> const std::vector<VkDescriptorSet>&;

    [[nodiscard]] auto is_input_valid(std::string_view p_name) const noexcept -> bool;
    [[nodiscard]] auto get_input_declaration(std::string_view p_name) const noexcept -> const render_pass_input_declaration*;

private:
    [[nodiscard]] auto is_invalidated(u32 p_set, u32 p_binding) const -> bool;

private:
	render_pass_specification m_specification{};
    vulkan_descriptor_set_manager m_descriptor_set_manager{};
};

} // end namespace kb::backend::vk

#endif
