#pragma once
#ifndef KABLUNK_PLATFORM_VULKAN_RENDER_PASS_H
#define KABLUNK_PLATFORM_VULKAN_RENDER_PASS_H

#include "Kablunk/Renderer/render_pass.h"

#include <vulkan/vulkan.h>

#include "Platform/Vulkan/vulkan_descriptor_set_manager.h"

namespace kb::render
{ // start namespace kb

class vulkan_render_pass final : public render_pass
{
public:
	vulkan_render_pass(const render_pass_specification& p_specification);
	~vulkan_render_pass() noexcept override = default;

	render_pass_specification& get_specification() override { return m_specification; }
	const render_pass_specification& get_specification() const override { return m_specification; }

    // #TODO concept template like vulkan_descriptor_set_manager...
    void set_input(std::string_view p_name, arc<uniform_buffer> p_uniform_buffer) noexcept override;
    void set_input(std::string_view p_name, arc<UniformBufferSet> p_uniform_buffer_set) noexcept override;
    void set_input(std::string_view p_name, arc<StorageBuffer> p_storage_buffer) noexcept override;
    void set_input(std::string_view p_name, arc<StorageBufferSet> p_storage_buffer_set) noexcept override;
    void set_input(std::string_view p_name, arc<Texture2D> p_texture_2d) noexcept override;
    void set_input(std::string_view p_name, arc<Image2D> p_image_2d) noexcept override;

    // Returns either a valid color attachment, depth attachment, or null
    arc<Image2D> get_output_image(u32 p_index) noexcept override;
    arc<Image2D> get_depth_output() noexcept override;
    u32 get_first_set_index() const override;
    arc<Pipeline> get_pipeline() const noexcept override;
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

} // end namespace kb

#endif
