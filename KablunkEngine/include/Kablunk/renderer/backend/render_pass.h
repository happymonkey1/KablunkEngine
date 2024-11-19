#pragma once
#ifndef KABLUNK_RENDERER_RENDER_PASS_H
#define KABLUNK_RENDERER_RENDER_PASS_H

#include "Kablunk/Core/Core.h"

#include "Kablunk/renderer/backend/frame_buffer.h"
#include "Kablunk/renderer/backend/pipeline.h"
#include "Kablunk/renderer/backend/storage_buffer.h"
#include "Kablunk/renderer/backend/storage_buffer_set.h"
#include "Kablunk/renderer/backend/uniform_buffer.h"
#include "Kablunk/renderer/backend/uniform_buffer_set.h"

#include <string_view>

namespace kb::render::backend
{ // start namespace kb::render::backend

struct render_pass_specification
{
    arc<pipeline> m_pipeline;
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
        arc<uniform_buffer_set> p_uniform_buffer_set
    ) noexcept = 0;

    virtual void set_input(
        std::string_view p_name,
        arc<storage_buffer> p_storage_buffer
    ) noexcept = 0;

    virtual void set_input(
        std::string_view p_name,
        arc<storage_buffer_set> p_storage_buffer_set
    ) noexcept = 0;

    virtual void set_input(
        std::string_view p_name,
        arc<texture_2d> p_texture_2d
    ) noexcept = 0;

    virtual void set_input(
        std::string_view p_name,
        arc<image_2d> p_image_2d
    ) noexcept = 0;

    virtual arc<image_2d> get_output_image(u32 p_index) noexcept = 0;
    virtual arc<image_2d> get_depth_output() noexcept = 0;
    virtual u32 get_first_set_index() const = 0;

    virtual arc<pipeline> get_pipeline() const noexcept = 0;
    virtual arc<frame_buffer> get_target_frame_buffer() const noexcept = 0;

    virtual bool validate() noexcept = 0;
    virtual void bake() noexcept = 0;
    virtual bool is_baked() const noexcept = 0;
    virtual void rt_prepare() noexcept = 0;

	static arc<render_pass> create(const render_pass_specification& specification) noexcept;
};

} // end namespace kb::render::backend

#endif
