#include "kablunkpch.h"

#include "Kablunk/Renderer/VertexArray.h"

#include "Kablunk/Renderer/Renderer.h"

namespace kb
{
	ref<VertexArray> VertexArray::Create()
	{
        KB_CORE_ASSERT(false, "Deprecated");
        return ref<VertexArray>{};
	}
}
