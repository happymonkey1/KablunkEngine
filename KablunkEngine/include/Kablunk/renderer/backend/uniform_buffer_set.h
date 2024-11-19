#pragma once
#ifndef KABLUNK_RENDERER_UNIFORM_BUFFER_SET_H
#define KABLUNK_RENDERER_UNIFORM_BUFFER_SET_H

#include "Kablunk/Core/Core.h"

#include "Kablunk/renderer/backend/uniform_buffer.h"

namespace kb::render::backend
{ // start namespace kb::render::backend

class uniform_buffer_set : public RefCounted
{
public:
	~uniform_buffer_set() override = default;

	virtual arc<uniform_buffer> get() = 0;
    virtual arc<uniform_buffer> rt_get() = 0;
    virtual arc<uniform_buffer> get(u32 p_frame) = 0;
	virtual void set(arc<uniform_buffer> p_uniform_buffer, u32 p_frame = 0) = 0;

	static arc<uniform_buffer_set> create(u32 p_size, u32 p_frames_in_flight);
};

} // end namespace kb::render::backend

#endif
