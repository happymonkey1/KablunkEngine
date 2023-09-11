#pragma once
#ifndef KABLUNK_RENDERER_RENDER_PASS_H
#define KABLUNK_RENDERER_RENDER_PASS_H

#include "Kablunk/Core/Core.h"

#include "Kablunk/Renderer/Framebuffer.h"

namespace Kablunk
{

	struct RenderPassSpecification
	{
		ref<Framebuffer> target_framebuffer;
		std::string debug_name;
	};

	class RenderPass : public RefCounted
	{
	public:
		virtual ~RenderPass() = default;

		virtual RenderPassSpecification& GetSpecification() = 0;
		virtual const RenderPassSpecification& GetSpecification() const = 0;

		static ref<RenderPass> Create(const RenderPassSpecification& specification);
	};

}

#endif
