#ifndef KB_OPENGL_RENDERERAPI_H
#define KB_OPENGL_RENDERERAPI_H

#include "Kablunk/Renderer/RendererAPI.h"

namespace Kablunk
{
	class OpenGLRendererAPI : public RendererAPI
	{
	public:
		virtual void Init() override;
		virtual void SetClearColor(const glm::vec4& color) override;
		virtual void Clear() override;

		virtual void DrawIndexed(const Ref<VertexArray>& vertexArray) override;
	};
	
}


#endif