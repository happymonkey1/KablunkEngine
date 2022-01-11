#ifndef KB_OPENGL_RENDERERAPI_H
#define KB_OPENGL_RENDERERAPI_H

#include "Kablunk/Renderer/RendererAPI.h"

namespace Kablunk
{
	class OpenGLRendererAPI : public RendererAPI
	{
	public:
		virtual void Init() override;
		
		virtual void BeginFrame() override {}
		virtual void EndFrame() override {}
		virtual void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) override;

		virtual void SetClearColor(const glm::vec4& color) override;
		virtual void Clear() override;

		virtual void ClearImage(IntrusiveRef<RenderCommandBuffer> commandBuffer, IntrusiveRef<Image2D> image) override { KB_CORE_ASSERT(false, "not implemented!") };
		virtual void DrawIndexed(const IntrusiveRef<VertexArray>& vertexArray, uint32_t indexCount = 0) override;

		virtual void SetWireframeMode(bool draw_wireframe) override { m_draw_wireframe = draw_wireframe; }
	private:
		bool m_draw_wireframe{ false };
	};
	
}


#endif
