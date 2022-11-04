#include "kablunkpch.h"

#include "Kablunk/Renderer/StorageBuffer.h"
#include "Kablunk/Renderer/RendererAPI.h"

#include "Platform/Vulkan/VulkanStorageBuffer.h"

namespace Kablunk
{
	IntrusiveRef<StorageBuffer> StorageBuffer::Create(size_t size, uint32_t binding)
	{
		switch (RendererAPI::GetAPI())
		{
		case RendererAPI::render_api_t::OpenGL:	KB_CORE_ASSERT(false, "not implemented!"); return nullptr;
		case RendererAPI::render_api_t::Vulkan:  return IntrusiveRef<VulkanStorageBuffer>::Create(size, binding);
		default:								KB_CORE_ASSERT(false, "Unknown RendererAPI!"); return nullptr;
		}
	}
}
