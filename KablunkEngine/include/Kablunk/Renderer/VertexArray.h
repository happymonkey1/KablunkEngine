#ifndef VERTEXARRAY_H
#define VERTEXARRAY_H


#include "Kablunk/Renderer/Buffer.h"
#include "Kablunk/Core/RefCounting.h"

#include <memory>

namespace kb
{
    class VertexArray : public RefCounted
	{
	public:
		virtual ~VertexArray() {}

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;

		virtual void AddVertexBuffer(const arc<VertexBuffer>& vertexBuffer) = 0;
		virtual void SetIndexBuffer(const arc<IndexBuffer>& indexBuffer) = 0;

		virtual const std::vector<arc<VertexBuffer>>& GetVertexBuffers() const = 0;
		virtual const arc<IndexBuffer>& GetIndexBuffer() const = 0;

		[[deprecated]] static arc<VertexArray> Create();
	};
}

#endif // 
