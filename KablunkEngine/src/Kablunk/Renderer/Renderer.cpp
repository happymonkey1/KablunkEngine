#include "kablunkpch.h"
#include "Kablunk/Renderer/Renderer.h"
#include "Kablunk/Renderer/RenderCommand2D.h"

#include "Platform/OpenGL/OpenGLShader.h"
#include "Platform/Vulkan/VulkanShader.h"

#include "Platform/Vulkan/VulkanContext.h"

#include "Kablunk/Renderer/RendererAPI.h"

#include "Platform/OpenGL/OpenGLRendererAPI.h"
#include "Platform/Vulkan/VulkanRendererAPI.h"

namespace Kablunk
{

	void Renderer::init()
	{
		KB_PROFILE_FUNCTION();

		m_shader_library = IntrusiveRef<ShaderLibrary>::Create();

		// initialize underlying renderer api
		switch (RendererAPI::GetAPI())
		{
			case RendererAPI::render_api_t::OpenGL: { m_renderer_api = new OpenGLRendererAPI{}; break; }
			case RendererAPI::render_api_t::Vulkan: { m_renderer_api = new VulkanRendererAPI{}; break; }
			default: { KB_CORE_ASSERT(false, "Unknown RendererAPI!"); break; }
		}

		KB_CORE_ASSERT(m_renderer_api, "RendererAPI not set?");
		m_renderer_api->Init();

		// Setting up data

		// Uniform buffers
		//m_SceneData->camera_uniform_buffer = UniformBuffer::Create(sizeof(SceneData::CameraData), 0);
		//m_SceneData->renderer_uniform_buffer = UniformBuffer::Create(sizeof(SceneData::RendererData), 1);
		//m_SceneData->point_lights_uniform_buffer = UniformBuffer::Create(sizeof(PointLightsData), 3);

		// Initialize freetype
		//if (FT_Init_FreeType(s_freetype_lib.get()))
		//	KB_CORE_ASSERT(false, "Could not initialize FreeType");
		
		render2d::init();
	}

	void Renderer::shutdown()
	{
		m_shader_dependencies.clear();
		
		m_shader_library.reset();

		render2d::shutdown();

		m_renderer_api->Shutdown();

		delete m_renderer_api;
	}

	IntrusiveRef<Texture2D> Renderer::GetWhiteTexture()
	{
		return render2d::get_white_texture();
	}

	IntrusiveRef<ShaderLibrary> Renderer::GetShaderLibrary()
	{
		return m_shader_library;
	}

	IntrusiveRef<Shader> Renderer::GetShader(const std::string& shader_name)
	{
		return m_shader_library->Get(shader_name);
	}

	void Renderer::RegisterShaderDependency(IntrusiveRef<Shader> shader, IntrusiveRef<Pipeline> pipeline)
	{
		m_shader_dependencies[shader->GetHash()].pipelines.push_back(pipeline);
	}

	void Renderer::RegisterShaderDependency(IntrusiveRef<Shader> shader, IntrusiveRef<Material> material)
	{
		m_shader_dependencies[shader->GetHash()].materials.push_back(material);
	}

	void Renderer::OnShaderReloaded(uint64_t hash)
	{
		if (m_shader_dependencies.find(hash) != m_shader_dependencies.end())
		{
			for (auto& material : m_shader_dependencies[hash].materials)
				material->Invalidate();

			for (auto& pipeline : m_shader_dependencies[hash].pipelines)
				pipeline->Invalidate();
		}
	}

	uint32_t Renderer::GetCurrentFrameIndex()
	{
		switch (RendererAPI::GetAPI())
		{
		case RendererAPI::render_api_t::Vulkan:	return VulkanContext::Get()->GetSwapchain().GetCurrentBufferIndex();
		default:								KB_CORE_ASSERT(false, "Unknown RenderAPI!"); return 0;
		}

	}

}
