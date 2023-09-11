#ifndef VERTEXARRAY_H
#define VERTEXARRAY_H


#include "Kablunk/Renderer/Buffer.h"
#include "Kablunk/Core/RefCounting.h"

#include <memory>

namespace Kablunk
{
	class VertexArray : public RefCounted
	{
	public:
		virtual ~VertexArray() {}

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;

		virtual void AddVertexBuffer(const ref<VertexBuffer>& vertexBuffer) = 0;
		virtual void SetIndexBuffer(const ref<IndexBuffer>& indexBuffer) = 0;

		virtual const std::vector<ref<VertexBuffer>>& GetVertexBuffers() const = 0;
		virtual const ref<IndexBuffer>& GetIndexBuffer() const = 0;

		static ref<VertexArray> Create();
	};
}

#endif // 
