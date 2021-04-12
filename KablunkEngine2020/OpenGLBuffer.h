#ifndef OpenGLBuffer_H
#define OpenGLBuffer_H

#include "Buffer.h"
#include "Renderer.h"

namespace kablunk
{
	/*
	* ========================================
	*            Vertex Buffer
	* ========================================
	*/
	class OpenGLVertexBuffer : public VertexBuffer
	{
	public:
		OpenGLVertexBuffer(float* vertices, uint32_t size);
		virtual ~OpenGLVertexBuffer();

		virtual void Bind() const;
		virtual void Unbind() const;

	private:
		Renderer::RendererID m_RendererID;
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

		virtual void Bind() const;
		virtual void Unbind() const;

		uint32_t GetCount() const override { return m_Count; }
	private:
		Renderer::RendererID m_RendererID;
		uint32_t m_Count;
	};
}

#endif
