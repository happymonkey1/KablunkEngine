#include "kablunkpch.h"
#include "Platform/OpenGL/OpenGLBuffer.h"

#include "glad/glad.h"
namespace kb
{
/*
* ========================================
*            Vertex Buffer
* ========================================
*/

OpenGLVertexBuffer::OpenGLVertexBuffer(uint32_t size)
{
#if 0
	glCreateBuffers(1, &m_RendererID);
	glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
	glBufferData(GL_ARRAY_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);
#endif
	glCreateBuffers(1, &m_renderer_id);
	glNamedBufferData(m_renderer_id, size, nullptr, GL_DYNAMIC_DRAW);

}


OpenGLVertexBuffer::OpenGLVertexBuffer(const void* data, uint32_t size)
{
#if 0
	glCreateBuffers(1, &m_RendererID);
	glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
	glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);
#endif
	glCreateBuffers(1, &m_renderer_id);
	glNamedBufferData(m_renderer_id, size, data, GL_STATIC_DRAW);
}

OpenGLVertexBuffer::~OpenGLVertexBuffer()
{
	glDeleteBuffers(1, &m_renderer_id);
}

void OpenGLVertexBuffer::SetData(const void* data, uint32_t size, uint32_t offset)
{
    KB_PROFILE_SCOPE;
#if 0
	glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
	glBufferSubData(GL_ARRAY_BUFFER, 0, size, data);
#endif
	glNamedBufferSubData(m_renderer_id, 0, size, data);
}

void OpenGLVertexBuffer::RT_SetData(const void* data, uint32_t size, uint32_t offset)
{
    KB_PROFILE_SCOPE;
	KB_CORE_WARN("OpenGLVertexBuffer RT_SetData not implemented!");
}

void OpenGLVertexBuffer::Bind() const
{
    KB_PROFILE_SCOPE;
	glBindBuffer(GL_ARRAY_BUFFER, m_renderer_id);
}

void OpenGLVertexBuffer::Unbind() const
{
    KB_PROFILE_SCOPE;

	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

/*
* ========================================
*            Index Buffer
* ========================================
*/

OpenGLIndexBuffer::OpenGLIndexBuffer(uint32_t count)
	: m_count{ count }
{
	glCreateBuffers(1, &m_renderer_id);
	glNamedBufferData(m_renderer_id, count * sizeof(uint32_t), nullptr, GL_STATIC_DRAW);
}

OpenGLIndexBuffer::OpenGLIndexBuffer(const void* data, uint32_t count)
	: m_count{count}
{
    KB_PROFILE_SCOPE;

#if 0
	glCreateBuffers(1, &m_RendererID);
	glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
	glBufferData(GL_ARRAY_BUFFER, count * sizeof(uint32_t), indices, GL_STATIC_DRAW);
#endif
	glCreateBuffers(1, &m_renderer_id);
	glNamedBufferData(m_renderer_id, count * sizeof(uint32_t), data, GL_STATIC_DRAW);
}

OpenGLIndexBuffer::~OpenGLIndexBuffer()
{
    KB_PROFILE_SCOPE;

	glDeleteBuffers(1, &m_renderer_id);
}

void OpenGLIndexBuffer::Bind() const
{
    KB_PROFILE_SCOPE;

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_renderer_id);
}

void OpenGLIndexBuffer::Unbind() const
{
    KB_PROFILE_SCOPE;

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void OpenGLIndexBuffer::SetData(const void* buffer, uint32_t size, uint32_t offset /* = 0 */)
{
	KB_CORE_WARN("OpenGlIndexBuffer SetData not implemented!");
}
}

