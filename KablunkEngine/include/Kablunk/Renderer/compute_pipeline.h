#pragma once
#ifndef KABLUNK_RENDERER_COMPUTE_PIPELINE_H
#define KABLUNK_RENDERER_COMPUTE_PIPELINE_H

#include "Kablunk/Core/RefCounting.h"

#include "Kablunk/Renderer/Shader.h"
#include "Kablunk/Renderer/RenderCommandBuffer.h"

namespace kb::render
{ // start namespace kb::render

class compute_pipeline : public kb::RefCounted
{
public:
    virtual ~compute_pipeline() = 0;

    // begin pass for the compute pipeline
    virtual void begin(kb::ref<kb::RenderCommandBuffer> p_render_command_buffer = {}) = 0;
    // begin pass on the render thread for the compute pipeline
    virtual void rt_begin(kb::ref<kb::RenderCommandBuffer> p_render_command_buffer = {}) = 0;
    // end pass for the compute pipeline
    virtual void end() = 0;

    virtual kb::ref<kb::Shader> get_shader() const = 0;

    // factory create method for a compute pipeline
    static kb::ref<compute_pipeline> create(kb::ref<kb::Shader> p_compute_shader);
};

} // end namespace kb::render

#endif
