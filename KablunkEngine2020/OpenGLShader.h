#ifndef OPENGLSHADER_H
#define OPENGLSHADER_H
#include "Shader.h"
#include "Renderer.h"

namespace kablunk
{
	class OpenGLShader : public Shader
	{
	public:
		OpenGLShader(const std::string& vertexSrc, const std::string& fragmentSrc);
		virtual ~OpenGLShader();

		virtual void Bind() const;
		virtual void Unbind() const;

	private:
		Renderer::RendererID m_RendererID;
	};
}


#endif