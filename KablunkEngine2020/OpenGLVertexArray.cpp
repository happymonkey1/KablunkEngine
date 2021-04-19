#include "kablunkpch.h"
#include "OpenGLVertexArray.h"

#include <glad/glad.h>

namespace Kablunk
{
	static GLenum ShaderDataTypeToOpenGLBaseType(ShaderDataType type)
	{
		switch (type)
		{
		case Kablunk::ShaderDataType::Float:    return GL_FLOAT;
		case Kablunk::ShaderDataType::Float2:   return GL_FLOAT;
		case Kablunk::ShaderDataType::Float3:   return GL_FLOAT;
		case Kablunk::ShaderDataType::Float4:   return GL_FLOAT;
		case Kablunk::ShaderDataType::Mat3:     return GL_FLOAT;
		case Kablunk::ShaderDataType::Mat4:     return GL_FLOAT;
		case Kablunk::ShaderDataType::Int:      return GL_INT;
		case Kablunk::ShaderDataType::Int2:     return GL_INT;
		case Kablunk::ShaderDataType::Int3:     return GL_INT;
		case Kablunk::ShaderDataType::Int4:     return GL_INT;
		case Kablunk::ShaderDataType::Bool:     return GL_BOOL;
		default:	                            KB_CORE_FATAL("Unkown ShaderDataType!"); return 0;
		}
	}


	OpenGLVertexArray::OpenGLVertexArray()
	{
		glCreateVertexArrays(1, &m_RendererID);
	}

	OpenGLVertexArray::~OpenGLVertexArray()
	{
		glDeleteVertexArrays(1, &m_RendererID);
	}

	void OpenGLVertexArray::Bind() const
	{
		glBindVertexArray(m_RendererID);
	}

	void OpenGLVertexArray::Unbind() const
	{
		glBindVertexArray(0);
	}

	void OpenGLVertexArray::AddVertexBuffer(const std::shared_ptr<VertexBuffer>& vertexBuffer)
	{
		//make sure buffer is bound before adding vertex buffer
		glBindVertexArray(m_RendererID);
		vertexBuffer->Bind();

		KB_CORE_ASSERT(vertexBuffer->GetLayout().GetElements().size(), "Vertex Buffer has no layout!");

		uint32_t index = 0;
		const auto& layout = vertexBuffer->GetLayout();
		for (const auto& element : layout)
		{
			glEnableVertexAttribArray(index);
			glVertexAttribPointer(
				index,
				element.GetComponentCount(),
				ShaderDataTypeToOpenGLBaseType(element.Type),
				element.Normalized ? GL_TRUE : GL_FALSE,
				layout.GetStride(),
				(const void*)element.Offset);
			index++;
		}

		m_VertexBuffers.push_back(vertexBuffer);
	}

	void OpenGLVertexArray::SetIndexBuffer(const std::shared_ptr<IndexBuffer>& indexBuffer)
	{
		glBindVertexArray(m_RendererID);
		indexBuffer->Bind();

		m_IndexBuffer = indexBuffer;
	}

}