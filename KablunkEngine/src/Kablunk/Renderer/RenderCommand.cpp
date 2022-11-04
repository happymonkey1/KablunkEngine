#include "kablunkpch.h"
#include "Kablunk/Renderer/RenderCommand.h"

#include "Platform/OpenGL/OpenGLRendererAPI.h"
#include "Platform/Vulkan/VulkanRendererAPI.h"

namespace Kablunk
{

	void RenderCommand::Init()
	{
		switch (RendererAPI::GetAPI())
		{
			case RendererAPI::render_api_t::OpenGL:
			{
				s_renderer_api = new OpenGLRendererAPI{};
				break;
			}
			case RendererAPI::render_api_t::Vulkan:
			{
				s_renderer_api = new VulkanRendererAPI{};
				break;
			}
			default:
			{
				KB_CORE_ASSERT(false, "Unknown RendererAPI!");
				break;
			}
		}

		s_renderer_api->Init();
	}

	static RenderCommandQueue s_resource_free_queue[3];

	RenderCommandQueue& RenderCommand::GetRenderResourceReleaseQueue(uint32_t index)
	{
		return s_resource_free_queue[index];
	}

	RenderCommandQueue& RenderCommand::GetRenderCommandQueue()
	{
		return Singleton<RenderCommandQueue>::get();
	}

}
