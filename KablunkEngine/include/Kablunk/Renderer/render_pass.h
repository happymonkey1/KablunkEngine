#pragma once
#ifndef KABLUNK_RENDERER_RENDER_PASS_H
#define KABLUNK_RENDERER_RENDER_PASS_H

#include "Kablunk/Core/Core.h"

#include "Kablunk/Renderer/frame_buffer.h"

#include <string_view>

#include "Kablunk/Renderer/Pipeline.h"
#include "Kablunk/Renderer/StorageBuffer.h"
#include "Kablunk/Renderer/StorageBufferSet.h"
#include "Kablunk/Renderer/uniform_buffer.h"
#include "Kablunk/Renderer/UniformBufferSet.h"

namespace kb::render
{ // start namespace kb::render

struct render_pass_specification
{
    arc<Pipeline> m_pipeline;
	std::string m_debug_name;
};

class render_pass : public RefCounted
{
public:
	~render_pass() override = default;

	virtual render_pass_specification& get_specification() = 0;
	virtual const render_pass_specification& get_specification() const = 0;

    virtual void set_input(
        std::string_view p_name,
        arc<uniform_buffer> p_uniform_buffer
    ) noexcept = 0;

    virtual void set_input(
        std::string_view p_name,
        arc<UniformBufferSet> p_uniform_buffer_set
    ) noexcept = 0;

    virtual void set_input(
        std::string_view p_name,
        arc<StorageBuffer> p_storage_buffer
    ) noexcept = 0;

    virtual void set_input(
        std::string_view p_name,
        arc<StorageBufferSet> p_storage_buffer_set
    ) noexcept = 0;

    virtual void set_input(
        std::string_view p_name,
        arc<Texture2D> p_texture_2d
    ) noexcept = 0;

    virtual void set_input(
        std::string_view p_name,
        arc<Image2D> p_image_2d
    ) noexcept = 0;

    virtual arc<Image2D> get_output_image(u32 p_index) noexcept = 0;
    virtual arc<Image2D> get_depth_output() noexcept = 0;
    virtual u32 get_first_set_index() const = 0;

    virtual arc<Pipeline> get_pipeline() const noexcept = 0;
    virtual arc<frame_buffer> get_target_frame_buffer() const noexcept = 0;

    virtual bool validate() noexcept = 0;
    virtual void bake() noexcept = 0;
    virtual bool is_baked() const noexcept = 0;
    virtual void rt_prepare() noexcept = 0;

	static arc<render_pass> create(const render_pass_specification& specification) noexcept;
};

} // end namespace kb::render

#endif
