#include "kablunkpch.h"

#include "Kablunk/Renderer/RendererAPI.h"
#include "Kablunk/Renderer/UniformBuffer.h"
#include "Platform/OpenGL/OpenGLUniformBuffer.h"
#include "Platform/Vulkan/VulkanUniformBuffer.h"

namespace Kablunk
{
	IntrusiveRef<UniformBuffer> UniformBuffer::Create(uint32_t size, uint32_t binding)
	{
		switch (RendererAPI::GetAPI())
		{
		case RendererAPI::render_api_t::None:	KB_CORE_ASSERT(false, "No renderer api set!"); return nullptr;
		case RendererAPI::render_api_t::OpenGL:	return IntrusiveRef<OpenGLUniformBuffer>::Create(size, binding);
		case RendererAPI::render_api_t::Vulkan:	return IntrusiveRef<VulkanUniformBuffer>::Create(size, binding);
		default:								KB_CORE_ASSERT(false, "Unknown render api set while trying to create uniform buffer!"); return nullptr;
		}
	}
}
