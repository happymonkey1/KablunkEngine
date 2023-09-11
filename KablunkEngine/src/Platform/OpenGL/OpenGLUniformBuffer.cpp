#include "kablunkpch.h"

#include <glad/glad.h>
#include "Platform/OpenGL/OpenGLUniformBuffer.h"


namespace kb
{
	OpenGLUniformBuffer::OpenGLUniformBuffer(uint32_t size, uint32_t binding)
		: m_size{ size }, m_binding{ binding }
	{
		m_local_storage = new uint8_t[size];
		
		glCreateBuffers(1, &m_renderer_id);
		glNamedBufferData(m_renderer_id, size, nullptr, GL_DYNAMIC_DRAW);
		glBindBufferBase(GL_UNIFORM_BUFFER, binding, m_renderer_id);
	}

	OpenGLUniformBuffer::~OpenGLUniformBuffer()
	{
		delete[] m_local_storage;

		glDeleteBuffers(1, &m_renderer_id);
	}

	void OpenGLUniformBuffer::SetData(const void* data, uint32_t size, uint32_t offest /*= 0*/)
	{
		memcpy(m_local_storage, data, size);
		glNamedBufferSubData(m_renderer_id, offest, size, data);
	}

}
