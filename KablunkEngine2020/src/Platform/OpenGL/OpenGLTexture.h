#ifndef KABLUNK_PLATFORM_OPENGL_TEXTURE_H
#define KABLUNK_PLATFORM_OPENGL_TEXTURE_H

#include "Kablunk/Renderer/Texture.h"
#include "Kablunk/Renderer/Renderer.h"
#include <string>

namespace Kablunk
{
	class OpenGLTexture2D : public Texture2D
	{
	public:
		OpenGLTexture2D(const std::string& path);
		~OpenGLTexture2D();
		virtual uint32_t GetWidth() const override { return m_Width; }
		virtual uint32_t GetHeight() const override { return m_Height; }


		virtual void Bind(uint32_t slot) const override;
	private:
		std::string m_Path;
		uint32_t m_Width;
		uint32_t m_Height;

		Renderer::RendererID m_RendererID;
	};
}

#endif