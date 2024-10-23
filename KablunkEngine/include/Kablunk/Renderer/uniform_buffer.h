#ifndef KABLUNK_RENDERER_UNIFORMBUFFER_H
#define KABLUNK_RENDERER_UNIFORMBUFFER_H

#include "Kablunk/Core/RefCounting.h"
#include "Kablunk/Renderer/Buffer.h"
#include "Kablunk/Core/CoreTypes.h"

namespace kb
{ // start namespace kb
class uniform_buffer : public RefCounted
{
public:
    ~uniform_buffer() override = default;
	virtual void set_data(const void* p_data, u32 p_size, u32 p_offset = 0) = 0;
	virtual void rt_set_data(const void* p_data, u32 p_size, u32 p_offset = 0) = 0;

	static arc<uniform_buffer> create(u32 p_size);
};
} // end namespace kb
#endif
