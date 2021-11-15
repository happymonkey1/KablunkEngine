#include "kablunkpch.h"
#include "Platform/OpenGL/OpenGLRendererAPI.h"

#include "glad/glad.h"

namespace Kablunk
{
	void OpenGLRendererAPI::Init()
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); //Interpolative blending
		//glBlendFunc(GL_ONE, GL_ONE); //Additive blending
		//glBlendFunc(GL_DST_COLOR, GL_ZERO); // Multiplicative blending

		glEnable(GL_MULTISAMPLE);
		glEnable(GL_STENCIL_TEST);

		glEnable(GL_DEPTH_TEST);

		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
	}

	void OpenGLRendererAPI::SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
	{
		glViewport(x, y, width, height);
	}

	void OpenGLRendererAPI::SetClearColor(const glm::vec4& color)
	{
		glClearColor(color.r, color.g, color.b, color.a);
	}

	void OpenGLRendererAPI::Clear()
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	void OpenGLRendererAPI::DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t indexCount)
	{
		// turn wireframe on
		if (m_draw_wireframe)
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

		vertexArray->Bind();
		uint32_t count = (indexCount) ? indexCount : vertexArray->GetIndexBuffer()->GetCount();
		glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, nullptr);
		//glBindTexture(GL_TEXTURE_2D, 0);

		// normal rendering
		if (m_draw_wireframe)
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

}
