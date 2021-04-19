#ifndef OPENGLSHADER_H
#define OPENGLSHADER_H
#include "Kablunk/Renderer/Shader.h"
#include "Kablunk/Renderer/Renderer.h"

namespace Kablunk
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