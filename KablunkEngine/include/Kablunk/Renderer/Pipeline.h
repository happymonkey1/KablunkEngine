#pragma once
#ifndef KABLUNK_RENDERER_PIPELINE_H
#define KABLUNK_RENDERER_PIPELINE_H

#include "Kablunk/Core/Core.h"
#include "Kablunk/Renderer/Shader.h"
#include "Kablunk/Renderer/Buffer.h"
#include "Kablunk/Renderer/uniform_buffer.h"
#include "Kablunk/Renderer/frame_buffer.h"

namespace kb::render
{ // start namespace kb::render
enum class PrimitiveTopology
{
	None = 0,
	Points,
	Lines,
	Triangles,
	LineStrip,
	TriangleStrip,
	TriangleFan
};

struct PipelineSpecification
{
	arc<Shader> shader;
    arc<frame_buffer> m_target_frame_buffer{};
	BufferLayout layout;
	BufferLayout instance_layout;
	PrimitiveTopology topology = PrimitiveTopology::Triangles;
	bool backface_culling = true;
	bool depth_test = true;
	bool depth_write = true;
	bool wireframe = false;

	std::string debug_name;
};

class Pipeline : public RefCounted
{
public:
    ~Pipeline() override = default;

	virtual PipelineSpecification& GetSpecification() = 0;
	virtual const PipelineSpecification& GetSpecification() const = 0;

	virtual void Invalidate() = 0;
    virtual arc<Shader> get_shader() const noexcept = 0;

	static arc<Pipeline> Create(const PipelineSpecification& specification);
};
} // end namespace kb::render

#endif
