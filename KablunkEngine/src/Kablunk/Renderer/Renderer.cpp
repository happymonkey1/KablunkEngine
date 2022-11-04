#include "kablunkpch.h"
#include "Kablunk/Renderer/Renderer.h"
#include "Kablunk/Renderer/RenderCommand2D.h"

#include "Platform/OpenGL/OpenGLShader.h"
#include "Platform/Vulkan/VulkanShader.h"

#include "Platform/Vulkan/VulkanContext.h"

#include "Kablunk/Renderer/RendererAPI.h"

namespace Kablunk
{
	IntrusiveRef<ShaderLibrary> Renderer::s_shader_library = IntrusiveRef<ShaderLibrary>::Create();

	struct ShaderDependencies
	{
		std::vector<IntrusiveRef<Pipeline>> pipelines;
		std::vector<IntrusiveRef<Material>> materials;
	};

	static std::unordered_map<uint64_t, ShaderDependencies> s_shader_dependencies;

	void Renderer::Init()
	{
		KB_PROFILE_FUNCTION();

		// Renderer initialization
		RenderCommand::Init();

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

	void Renderer::Shutdown()
	{
		s_shader_dependencies.clear();
		
		s_shader_library.reset();

		render2d::shutdown();

		RenderCommand::Shutdown();
	}

	IntrusiveRef<Texture2D> Renderer::GetWhiteTexture()
	{
		return render2d::get_white_texture();
	}

	IntrusiveRef<ShaderLibrary> Renderer::GetShaderLibrary()
	{
		return s_shader_library;
	}

	IntrusiveRef<Shader> Renderer::GetShader(const std::string& shader_name)
	{
		return s_shader_library->Get(shader_name);
	}

	void Renderer::OnWindowResize(uint32_t width, uint32_t height)
	{
		RenderCommand::SetViewport(0, 0, width, height);
	}

	void Renderer::RegisterShaderDependency(IntrusiveRef<Shader> shader, IntrusiveRef<Pipeline> pipeline)
	{
		s_shader_dependencies[shader->GetHash()].pipelines.push_back(pipeline);
	}

	void Renderer::RegisterShaderDependency(IntrusiveRef<Shader> shader, IntrusiveRef<Material> material)
	{
		s_shader_dependencies[shader->GetHash()].materials.push_back(material);
	}

	void Renderer::OnShaderReloaded(uint64_t hash)
	{
		if (s_shader_dependencies.find(hash) != s_shader_dependencies.end())
		{
			for (auto& material : s_shader_dependencies[hash].materials)
				material->Invalidate();

			for (auto& pipeline : s_shader_dependencies[hash].pipelines)
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
