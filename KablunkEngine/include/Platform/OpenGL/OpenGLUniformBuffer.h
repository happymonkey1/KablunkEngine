#ifndef KABLUNK_PLATFORM_OPENGL_UNIFORMBUFFER_H
#define KABLUNK_PLATFORM_OPENGL_UNIFORMBUFFER_H

#include "Kablunk/Renderer/RendererTypes.h"
#include "Kablunk/Renderer/UniformBuffer.h"
namespace Kablunk
{

	class OpenGLUniformBuffer : public UniformBuffer
	{
	public:
		OpenGLUniformBuffer(uint32_t size, uint32_t binding);
		virtual ~OpenGLUniformBuffer();

		virtual void SetData(const void* data, uint32_t size, uint32_t offest = 0) override;
		virtual void RT_SetData(const void* data, uint32_t size, uint32_t offset = 0) override { KB_CORE_ASSERT(false, "not implemented!"); }

		virtual RendererID GetBinding() const override { return m_renderer_id; }
	private:
		RendererID m_renderer_id;
		uint32_t m_size;
		uint32_t m_binding;
		uint8_t* m_local_storage;
	};

}
#endif
