#include "kablunkpch.h"
#include "Platform/OpenGL/OpenGLBuffer.h"

#include "glad/glad.h"
namespace Kablunk
{
	/*
	* ========================================
	*            Vertex Buffer
	* ========================================
	*/

	OpenGLVertexBuffer::OpenGLVertexBuffer(uint32_t size)
	{
		KB_PROFILE_FUNCTION();

#if 0
		glCreateBuffers(1, &m_RendererID);
		glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
		glBufferData(GL_ARRAY_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);
#endif
		glCreateBuffers(1, &m_RendererID);
		glNamedBufferData(m_RendererID, size, nullptr, GL_DYNAMIC_DRAW);

	}


	OpenGLVertexBuffer::OpenGLVertexBuffer(const void* data, uint32_t size)
	{
		KB_PROFILE_FUNCTION();

#if 0
		glCreateBuffers(1, &m_RendererID);
		glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
		glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);
#endif
		glCreateBuffers(1, &m_RendererID);
		glNamedBufferData(m_RendererID, size, data, GL_STATIC_DRAW);
	}

	OpenGLVertexBuffer::~OpenGLVertexBuffer()
	{
		KB_PROFILE_FUNCTION();

		glDeleteBuffers(1, &m_RendererID);
	}

	void OpenGLVertexBuffer::SetData(const void* data, uint32_t size, uint32_t offset)
	{
#if 0
		glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
		glBufferSubData(GL_ARRAY_BUFFER, 0, size, data);
#endif
		glNamedBufferSubData(m_RendererID, 0, size, data);
	}

	void OpenGLVertexBuffer::RT_SetData(const void* data, uint32_t size, uint32_t offset)
	{
		KB_CORE_WARN("OpenGLVertexBuffer RT_SetData not implemented!");
	}

	void OpenGLVertexBuffer::Bind() const
	{
		KB_PROFILE_FUNCTION();

		glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
	}

	void OpenGLVertexBuffer::Unbind() const
	{
		KB_PROFILE_FUNCTION();

		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	/*
	* ========================================
	*            Index Buffer
	* ========================================
	*/

	OpenGLIndexBuffer::OpenGLIndexBuffer(uint32_t count)
		: m_Count{ count }
	{
		KB_PROFILE_FUNCTION();

		glCreateBuffers(1, &m_RendererID);
		glNamedBufferData(m_RendererID, count * sizeof(uint32_t), nullptr, GL_STATIC_DRAW);
	}

	OpenGLIndexBuffer::OpenGLIndexBuffer(const void* data, uint32_t count)
		: m_Count{count}
	{
		KB_PROFILE_FUNCTION();
#if 0
		glCreateBuffers(1, &m_RendererID);
		glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
		glBufferData(GL_ARRAY_BUFFER, count * sizeof(uint32_t), indices, GL_STATIC_DRAW);
#endif
		glCreateBuffers(1, &m_RendererID);
		glNamedBufferData(m_RendererID, count * sizeof(uint32_t), data, GL_STATIC_DRAW);
	}

	OpenGLIndexBuffer::~OpenGLIndexBuffer()
	{
		KB_PROFILE_FUNCTION();

		glDeleteBuffers(1, &m_RendererID);
	}

	void OpenGLIndexBuffer::Bind() const
	{
		KB_PROFILE_FUNCTION();

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererID);
	}

	void OpenGLIndexBuffer::Unbind() const
	{
		KB_PROFILE_FUNCTION();

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}

	void OpenGLIndexBuffer::SetData(const void* buffer, uint32_t size, uint32_t offset /* = 0 */)
	{
		KB_CORE_WARN("OpenGlIndexBuffer SetData not implemented!");
	}
}

