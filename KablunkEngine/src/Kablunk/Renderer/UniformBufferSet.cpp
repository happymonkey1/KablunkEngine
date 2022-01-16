#include "kablunkpch.h"

#include "Kablunk/Renderer/UniformBufferSet.h"
#include "Kablunk/Renderer/RendererAPI.h"

#include "Platform/Vulkan/VulkanUniformBufferSet.h"

namespace Kablunk
{
	IntrusiveRef<UniformBufferSet> UniformBufferSet::Create(uint32_t frames)
	{
		switch (RendererAPI::GetAPI())
		{
		case RendererAPI::RenderAPI_t::OpenGL:	KB_CORE_ASSERT(false, "not implemented!"); return nullptr;
		case RendererAPI::RenderAPI_t::Vulkan:	return IntrusiveRef<VulkanUniformBufferSet>::Create(frames);
		default:								KB_CORE_ASSERT(false, "Unknown RendererAPI!"); return nullptr;
		}
	}
}
