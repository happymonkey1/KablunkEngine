#ifndef KABLUNK_RENDERER_OPENGLBUFFER_H
#define KABLUNK_RENDERER_OPENGLBUFFER_H

#include "Kablunk/Renderer/Buffer.h"
#include "Kablunk/Renderer/Renderer.h"

namespace kb
{
	/*
	* ========================================
	*            Vertex Buffer
	* ========================================
	*/
	class OpenGLVertexBuffer : public VertexBuffer
	{
	public:
		OpenGLVertexBuffer(uint32_t size);
		OpenGLVertexBuffer(const void* data, uint32_t size);
		virtual ~OpenGLVertexBuffer();

		virtual void SetData(const void* data, uint32_t size, uint32_t offset = 0) override;
		virtual void RT_SetData(const void* data, uint32_t size, uint32_t offset = 0) override;

		virtual void Bind() const override;
		virtual void Unbind() const override;

		virtual const BufferLayout& GetLayout() const override { return m_layout; }
		virtual void SetLayout(const BufferLayout& layout) override { m_layout = layout; }

		virtual RendererID GetRendererID() const override { return m_renderer_id; }
	private:
		RendererID m_renderer_id;
		BufferLayout m_layout;
	};

	/*
	* ========================================
	*            Index Buffer
	* ========================================
	*/
	class OpenGLIndexBuffer : public IndexBuffer
	{
	public:
		OpenGLIndexBuffer(uint32_t count);
		OpenGLIndexBuffer(const void* data, uint32_t count);
		virtual ~OpenGLIndexBuffer();

		virtual void Bind() const override;
		virtual void Unbind() const override;

		virtual void SetData(const void* buffer, uint32_t size, uint32_t offset /* = 0 */) override;

		virtual const uint32_t GetCount() const override { return m_count; }
		virtual uint32_t GetSize() const override { return m_count * sizeof(uint32_t); }
		virtual RendererID GetRendererID() const override { return m_renderer_id; }
	private:
		RendererID m_renderer_id;
		uint32_t m_count;
	};
}

#endif
