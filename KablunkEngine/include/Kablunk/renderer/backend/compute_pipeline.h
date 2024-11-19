#pragma once
#ifndef KABLUNK_RENDERER_COMPUTE_PIPELINE_H
#define KABLUNK_RENDERER_COMPUTE_PIPELINE_H

#include "Kablunk/Core/RefCounting.h"

#include "Kablunk/Renderer/backend/shader.h"
#include "Kablunk/Renderer/backend/render_command_buffer.h"

namespace kb::render::backend
{ // start namespace kb::render::backend

class compute_pipeline : public RefCounted
{
public:
    virtual ~compute_pipeline() = 0;

    // begin pass for the compute pipeline
    virtual void begin(arc<render_command_buffer> p_render_command_buffer = {}) = 0;
    // begin pass on the render thread for the compute pipeline
    virtual void rt_begin(arc<render_command_buffer> p_render_command_buffer = {}) = 0;
    // end pass for the compute pipeline
    virtual void end() = 0;

    virtual arc<shader> get_shader() const = 0;

    // factory create method for a compute pipeline
    static arc<compute_pipeline> create(arc<shader> p_compute_shader);
};

} // end namespace kb::render::backend

#endif
