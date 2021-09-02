#include "kablunkpch.h"

#include "RendererAPI.h"
#include "Kablunk/Renderer/UniformBuffer.h"
#include "Platform/OpenGL/OpenGLUniformBuffer.h"

namespace Kablunk
{
	Ref<UniformBuffer> UniformBuffer::Create(uint32_t size, uint32_t binding)
	{
		switch (RendererAPI::GetAPI())
		{
		case RendererAPI::RenderAPI_t::OpenGL:	return CreateRef<OpenGLUniformBuffer>(size, binding);
		case RendererAPI::RenderAPI_t::None:	KB_CORE_ASSERT(false, "No renderer api set!"); return nullptr;
		default:								KB_CORE_ASSERT(false, "Unknown render api set while trying to create uniform buffer!"); return nullptr;
		}
	}
}
