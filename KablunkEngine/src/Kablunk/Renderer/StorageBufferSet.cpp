#include "kablunkpch.h"

#include "Kablunk/Renderer/StorageBufferSet.h"
#include "Kablunk/Renderer/RendererAPI.h"

#include "Platform/Vulkan/VulkanStorageBufferSet.h"

namespace kb
{
ref<StorageBufferSet> StorageBufferSet::Create(uint32_t frames)
{
	switch (RendererAPI::GetAPI())
	{
	case RendererAPI::render_api_t::Vulkan:	return static_cast<ref<StorageBufferSet>>(ref<VulkanStorageBufferSet>::Create(frames));
	default:								KB_CORE_ASSERT(false, "Unknown RendererAPI!"); return ref<StorageBufferSet>{};
	}
}
}
