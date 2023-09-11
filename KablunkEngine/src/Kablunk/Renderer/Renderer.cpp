#include "kablunkpch.h"
#include "Kablunk/Renderer/Renderer.h"
#include "Kablunk/Renderer/RenderCommand2D.h"

#include "Platform/OpenGL/OpenGLShader.h"
#include "Platform/Vulkan/VulkanShader.h"

#include "Platform/Vulkan/VulkanContext.h"

#include "Kablunk/Renderer/RendererAPI.h"

#include "Platform/OpenGL/OpenGLRendererAPI.h"
#include "Platform/Vulkan/VulkanRendererAPI.h"

#include "Kablunk/Core/Application.h"

namespace Kablunk
{

	void Renderer::init()
	{
        KB_PROFILE_FUNC();

		// initialize render command queues
		for (size_t i = 0; i < s_render_command_queue_size; ++i)
			m_command_queues[i] = kb::render_command_queue{};

		m_shader_library = IntrusiveRef<ShaderLibrary>::Create();

		// ==========
		// 3d shaders
		// ==========
        m_shader_library->Load("resources/shaders/Kablunk_diffuse_static.glsl");
		m_shader_library->Load("resources/shaders/scene_composite.glsl");

		// ==========
		// 2d shaders
		// ==========
        m_shader_library->Load("resources/shaders/Renderer2D_Circle.glsl");
        m_shader_library->Load("resources/shaders/Renderer2D_Quad.glsl");
        m_shader_library->Load("resources/shaders/Renderer2D_Line.glsl");
        m_shader_library->Load("resources/shaders/Renderer2D_UI.glsl");
        m_shader_library->Load("resources/shaders/Renderer2D_Text.glsl");
		// ==========

		// compile shaders that were submitted
		Application::Get().get_render_thread().pump();

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

		//render2d::init();
	}

	void Renderer::shutdown()
	{
		m_shader_dependencies.clear();
		
		m_shader_library.reset();

		render2d::shutdown();

		m_renderer_api->Shutdown();

		// shutdown vulkan context
		//VulkanContext::Get()->Shutdown();

        
		if (m_command_queues)
		{
			for (size_t i = 0; i < s_render_command_queue_size; ++i)
                if (!m_command_queues[i].is_empty())
			        KB_CORE_WARN("[renderer]: renderer shutting down but command_queue[{}] is not empty?", i);
		}

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

    void Renderer::register_shader_dependency(IntrusiveRef<Shader> p_shader, IntrusiveRef<kb::render::compute_pipeline> p_compute_pipeline)
    {
        m_shader_dependencies[p_shader->GetHash()].compute_pipelines.push_back(p_compute_pipeline);
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

	void Renderer::wait_and_render(render_thread* rendering_thread)
	{
        KB_PROFILE_FUNC()
		KB_CORE_ASSERT(rendering_thread, "render thread is null?");
		
		{
			// #TODO add profiling
			rendering_thread->wait_and_set(thread_state_t::kick, thread_state_t::busy);
		}
		// execute command queue
		m_command_queues[get_render_command_queue_index()].execute();
		rendering_thread->set(thread_state_t::idle);
	}

	// main render function which runs on render thread
	void Renderer::render_thread_func(render_thread* rendering_thread)
	{
		while (rendering_thread->is_running())
			wait_and_render(rendering_thread);
	}

	void Renderer::swap_queues()
	{
		m_render_command_queue_submission_index = (m_render_command_queue_submission_index + 1) % s_render_command_queue_size;
	}

}
