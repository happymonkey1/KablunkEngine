#pragma once
#ifndef KABLUNK_RENDERER_BACKEND_PIPELINE_H
#define KABLUNK_RENDERER_BACKEND_PIPELINE_H

#include "Kablunk/Core/Core.h"
#include "Kablunk/renderer/backend/shader.h"
#include "Kablunk/renderer/backend/buffer.h"
#include "Kablunk/renderer/backend/frame_buffer.h"

namespace kb::render::backend
{ // start namespace kb::render::backend

enum class primitive_topology_t
{
	none = 0,
	points,
	lines,
	triangles,
	line_strip,
	triangle_strip,
	triangle_fan
};

struct pipeline_specification_t
{
	arc<shader> shader;
    arc<frame_buffer> m_target_frame_buffer{};
	BufferLayout layout;
	BufferLayout instance_layout;
	primitive_topology_t topology = primitive_topology_t::triangles;
	bool backface_culling = true;
	bool depth_test = true;
	bool depth_write = true;
	bool wireframe = false;

	std::string debug_name;
};

class pipeline : public RefCounted
{
public:
    ~pipeline() override = default;

	virtual pipeline_specification_t& get_specification() = 0;
	virtual const pipeline_specification_t& get_specification() const = 0;

	virtual void invalidate() = 0;
    virtual arc<shader> get_shader() const noexcept = 0;

	static arc<pipeline> create(const pipeline_specification_t& specification);
};

} // end namespace kb::render::backend

#endif
