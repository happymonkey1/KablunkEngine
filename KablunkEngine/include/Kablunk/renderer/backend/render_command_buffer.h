#pragma once
#ifndef KABLUNK_RENDERER_COMMAND_BUFFER_H
#define KABLUNK_RENDERER_COMMAND_BUFFER_H

namespace kb::render::backend
{ // start namespace kb::render::backend

class render_command_buffer : public RefCounted
{
public:
    ~render_command_buffer() override = default;

	virtual void begin() = 0;
	virtual void end() = 0;
	virtual void submit() = 0;

	virtual float get_execution_gpu_time(uint32_t frame_index, uint32_t query_index = 0) const = 0;
	//virtual const PipelineStatistics& GetPipelineStatistics(uint32_t frame_index) const = 0;

	virtual uint64_t begin_timestamp_query() = 0;
	virtual void end_timestamp_query(uint64_t query_index) = 0;

	static arc<render_command_buffer> create(uint32_t count = 0, const std::string& debug_name = "");
	static arc<render_command_buffer> create_from_swap_chain(const std::string& debug_name = "");
};

} // end namespace kb::render::backend

#endif
