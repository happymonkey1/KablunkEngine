#include "kablunkpch.h"

#include "Kablunk/Renderer/Renderer.h"
#include "Platform/Vulkan/VulkanRendererAPI.h"

namespace Kablunk
{

	void VulkanRendererAPI::Init()
	{
		Renderer::GetShaderLibrary()->Load("resources/shaders/Kablunk_diffuse_static.glsl");

		Renderer::GetShaderLibrary()->Load("resources/shaders/Renderer2D_Circle.glsl");
		Renderer::GetShaderLibrary()->Load("resources/shaders/Renderer2D_Quad.glsl");

		// compile shaders that were submitted
		RenderCommand::WaitAndRender();
	}

	void VulkanRendererAPI::SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
	{

	}

	void VulkanRendererAPI::SetClearColor(const glm::vec4& color)
	{

	}

	void VulkanRendererAPI::Clear()
	{

	}

	void VulkanRendererAPI::DrawIndexed(const IntrusiveRef<VertexArray>& vertexArray, uint32_t indexCount /*= 0*/)
	{

	}

}
