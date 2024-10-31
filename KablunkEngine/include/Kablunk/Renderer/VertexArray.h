#ifndef VERTEXARRAY_H
#define VERTEXARRAY_H


#include "Kablunk/Renderer/backend/buffer.h"
#include "Kablunk/Core/RefCounting.h"

#include <memory>

namespace kb::render::backend
{ // start namespace kb::render::backend
class VertexArray : public RefCounted
{
public:
	virtual ~VertexArray() {}

	virtual void Bind() const = 0;
	virtual void Unbind() const = 0;

	virtual void AddVertexBuffer(const arc<vertex_buffer>& vertexBuffer) = 0;
	virtual void SetIndexBuffer(const arc<index_buffer>& indexBuffer) = 0;

	virtual const std::vector<arc<vertex_buffer>>& GetVertexBuffers() const = 0;
	virtual const arc<index_buffer>& GetIndexBuffer() const = 0;

	[[deprecated]] static arc<VertexArray> Create();
};
} // end namespace kb::render::backend

#endif 
