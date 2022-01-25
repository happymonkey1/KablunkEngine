#include "kablunkpch.h"

#include "Kablunk/Renderer/StorageBufferSet.h"
#include "Kablunk/Renderer/RendererAPI.h"

#include "Platform/Vulkan/VulkanStorageBufferSet.h"

namespace Kablunk
{
	IntrusiveRef<StorageBufferSet> StorageBufferSet::Create(uint32_t frames)
	{
		switch (RendererAPI::GetAPI())
		{
		case RendererAPI::RenderAPI_t::OpenGL:	KB_CORE_ASSERT(false, "not implemented!"); return nullptr;
		case RendererAPI::RenderAPI_t::Vulkan:	return IntrusiveRef<VulkanStorageBufferSet>::Create(frames);
		default:								KB_CORE_ASSERT(false, "Unknown RendererAPI!"); return nullptr;
		}
	}
}