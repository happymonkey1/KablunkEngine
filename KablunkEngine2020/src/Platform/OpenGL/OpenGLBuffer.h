#ifndef OpenGLBuffer_H
#define OpenGLBuffer_H

#include "Kablunk/Renderer/Buffer.h"
#include "Kablunk/Renderer/Renderer.h"

namespace Kablunk
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
		OpenGLVertexBuffer(float* vertices, uint32_t size);
		virtual ~OpenGLVertexBuffer();

		virtual void Bind() const override;
		virtual void Unbind() const override;

		virtual const BufferLayout& GetLayout() const override { return m_Layout; }
		virtual void SetLayout(const BufferLayout& layout) override { m_Layout = layout; }

	private:
		Renderer::RendererID m_RendererID;
		BufferLayout m_Layout;
	};

	/*
	* ========================================
	*            Index Buffer
	* ========================================
	*/
	class OpenGLIndexBuffer : public IndexBuffer
	{
	public:
		OpenGLIndexBuffer(uint32_t* indices, uint32_t count);
		virtual ~OpenGLIndexBuffer();

		virtual void Bind() const override;
		virtual void Unbind() const override;

		virtual const uint32_t GetCount() const override { return m_Count; }
	private:
		Renderer::RendererID m_RendererID;
		uint32_t m_Count;
	};
}

#endif
