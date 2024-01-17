#include "kablunkpch.h"

#include "Kablunk/Renderer/RendererAPI.h"
#include "Kablunk/Renderer/UniformBuffer.h"
#include "Platform/Vulkan/VulkanUniformBuffer.h"

namespace kb
{
	ref<UniformBuffer> UniformBuffer::Create(uint32_t size, uint32_t binding)
	{
		switch (RendererAPI::GetAPI())
		{
		case RendererAPI::render_api_t::None:	KB_CORE_ASSERT(false, "No renderer api set!"); return ref<UniformBuffer>{};
		case RendererAPI::render_api_t::Vulkan:	return static_cast<ref<UniformBuffer>>(ref<VulkanUniformBuffer>::Create(size, binding));
		default:								KB_CORE_ASSERT(false, "Unknown render api set while trying to create uniform buffer!"); return ref<UniformBuffer>{};
		}
	}
}
