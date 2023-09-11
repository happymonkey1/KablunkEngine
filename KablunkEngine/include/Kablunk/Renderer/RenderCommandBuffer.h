#pragma once
#ifndef KABLUNK_RENDERER_RENDER_COMMAND_BUFFER_H
#define KABLUNK_RENDERER_RENDER_COMMAND_BUFFER_H

namespace Kablunk
{
	class RenderCommandBuffer : public RefCounted
	{
	public:
		virtual ~RenderCommandBuffer() {}

		virtual void Begin() = 0;
		virtual void End() = 0;
		virtual void Submit() = 0;

		virtual float GetExecutionGPUTime(uint32_t frame_index, uint32_t query_index = 0) const = 0;
		//virtual const PipelineStatistics& GetPipelineStatistics(uint32_t frame_index) const = 0;

		virtual uint64_t BeginTimestampQuery() = 0;
		virtual void EndTimestampQuery(uint64_t query_index) = 0;

		static ref<RenderCommandBuffer> Create(uint32_t count = 0, const std::string& debug_name = "");
		static ref<RenderCommandBuffer> CreateFromSwapChain(const std::string& debug_name = "");
	};
}

#endif
