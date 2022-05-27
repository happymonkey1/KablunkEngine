#pragma once
#ifndef KABLUNK_RENDERER_PIPELINE_H
#define KABLUNK_RENDERER_PIPELINE_H

#include "Kablunk/Core/Core.h"
#include "Kablunk/Renderer/Shader.h"
#include "Kablunk/Renderer/Buffer.h"
#include "Kablunk/Renderer/UniformBuffer.h"

#include "Kablunk/Renderer/RenderPass.h"

namespace Kablunk
{
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
		IntrusiveRef<Shader> shader;
		BufferLayout layout;
		BufferLayout instance_layout;
		IntrusiveRef<RenderPass> render_pass;
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
		virtual ~Pipeline() = default;

		virtual PipelineSpecification& GetSpecification() = 0;
		virtual const PipelineSpecification& GetSpecification() const = 0;

		virtual void Invalidate() = 0;
		virtual void SetUniformBuffer(IntrusiveRef<UniformBuffer> uniform_buffer, uint32_t binding, uint32_t set = 0) = 0;

		static IntrusiveRef<Pipeline> Create(const PipelineSpecification& specification);
	};
}

#endif
