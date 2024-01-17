#include "kablunkpch.h"

#include "Kablunk/Renderer/StorageBuffer.h"
#include "Kablunk/Renderer/RendererAPI.h"

#include "Platform/Vulkan/VulkanStorageBuffer.h"

namespace kb
{
	ref<StorageBuffer> StorageBuffer::Create(size_t size, uint32_t binding)
	{
		switch (RendererAPI::GetAPI())
		{
		case RendererAPI::render_api_t::Vulkan:  return static_cast<ref<StorageBuffer>>(ref<VulkanStorageBuffer>::Create(size, binding));
		default:								KB_CORE_ASSERT(false, "Unknown RendererAPI!"); return ref<StorageBuffer>{};
		}
	}
}
