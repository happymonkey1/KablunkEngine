#pragma once

#include "Kablunk/Renderer/VertexArray.h"
#include "Kablunk/Renderer/Renderer.h"


namespace Kablunk
{
	class OpenGLVertexArray : public VertexArray
	{
	public:
		OpenGLVertexArray();
		virtual ~OpenGLVertexArray();

		virtual void Bind() const override;
		virtual void Unbind() const override;

		virtual void AddVertexBuffer(const ref<VertexBuffer>& vertexBuffer) override;
		virtual void SetIndexBuffer(const ref<IndexBuffer>& indexBuffer) override;

		virtual const std::vector<ref<VertexBuffer>>& GetVertexBuffers() const { return m_VertexBuffers; };
		virtual const ref<IndexBuffer>& GetIndexBuffer() const { return m_IndexBuffer; };

	private:
		std::vector<ref<VertexBuffer>> m_VertexBuffers;
		ref<IndexBuffer> m_IndexBuffer;
		RendererID m_RendererID;
		uint32_t m_VertexBufferIndex = 0;
	};
}
