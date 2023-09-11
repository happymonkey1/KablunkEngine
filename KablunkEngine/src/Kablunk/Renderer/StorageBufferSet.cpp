#include "kablunkpch.h"

#include "Kablunk/Renderer/StorageBufferSet.h"
#include "Kablunk/Renderer/RendererAPI.h"

#include "Platform/Vulkan/VulkanStorageBufferSet.h"

namespace Kablunk
{
	ref<StorageBufferSet> StorageBufferSet::Create(uint32_t frames)
	{
		switch (RendererAPI::GetAPI())
		{
		case RendererAPI::render_api_t::OpenGL:	KB_CORE_ASSERT(false, "not implemented!"); return nullptr;
		case RendererAPI::render_api_t::Vulkan:	return ref<VulkanStorageBufferSet>::Create(frames);
		default:								KB_CORE_ASSERT(false, "Unknown RendererAPI!"); return nullptr;
		}
	}
}
