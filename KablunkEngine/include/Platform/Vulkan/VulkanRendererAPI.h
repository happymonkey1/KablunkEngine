#pragma once
#ifndef KABLUNK_PLATFORM_VULKAN_RENDER_API_H
#define KABLUNK_PLATFORM_VULKAN_RENDER_API_H

#include "Kablunk/Core/RefCounting.h"
#include "Kablunk/Renderer/RendererAPI.h"

namespace Kablunk
{
	class VulkanRendererAPI : public RendererAPI
	{
	public:
		virtual void Init() override;
		virtual void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) override;

		virtual void SetClearColor(const glm::vec4& color) override;
		virtual void Clear() override;

		virtual void DrawIndexed(const IntrusiveRef<VertexArray>& vertexArray, uint32_t indexCount = 0) override;

		virtual void SetWireframeMode(bool draw_wireframe) override { m_draw_wireframe = draw_wireframe; }
	private:
		bool m_draw_wireframe{ false };
	};
}

#endif
