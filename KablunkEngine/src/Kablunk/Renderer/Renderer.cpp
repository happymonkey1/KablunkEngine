#include "kablunkpch.h"
#include "Kablunk/Renderer/Renderer.h"

#include "kablunk/renderer/backend/vulkan/vulkan_shader.h"
#include "kablunk/renderer/backend/vulkan/vulkan_context.h"

#include "Kablunk/Core/Application.h"
#include "Kablunk/Core/Timers.h"

namespace kb::render
{ // start namespace kb::render
void Renderer::init()
{
    KB_PROFILE_SCOPE;

	// initialize render command queues
	for (size_t i = 0; i < s_render_command_queue_size; ++i)
		m_command_queues[i] = backend::render_command_queue{};

	m_shader_library = arc<shader_library>::Create();

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

    m_backend.init();
}

void Renderer::shutdown()
{
    KB_PROFILE_SCOPE;

	m_shader_dependencies.clear();

	m_shader_library.reset();

	// render2d::shutdown();

	m_backend.shutdown();

	// shutdown vulkan context
	//VulkanContext::Get()->Shutdown();

	for (size_t i = 0; i < s_render_command_queue_size; ++i)
        if (!m_command_queues[i].is_empty())
			KB_CORE_WARN("[renderer]: renderer shutting down but command_queue[{}] is not empty?", i);
}

arc<shader_library> Renderer::GetShaderLibrary()
{
	return m_shader_library;
}

arc<backend::shader> Renderer::GetShader(const std::string& p_shader_name)
{
	return m_shader_library->Get(p_shader_name);
}

void Renderer::register_shader_dependency(arc<backend::shader> p_shader, arc<backend::pipeline> p_pipeline)
{
	m_shader_dependencies[p_shader->get_hash()].pipelines.push_back(p_pipeline);
}

void Renderer::register_shader_dependency(arc<backend::shader> p_shader, arc<backend::material> p_material)
{
	m_shader_dependencies[p_shader->get_hash()].materials.push_back(p_material);
}

void Renderer::register_shader_dependency(
    arc<backend::shader> p_shader,
    arc<backend::compute_pipeline> p_compute_pipeline
)
{
    m_shader_dependencies[p_shader->get_hash()].compute_pipelines.push_back(p_compute_pipeline);
}

void Renderer::on_shader_reloaded(const uint64_t p_hash)
{
	if (m_shader_dependencies.find(p_hash) != m_shader_dependencies.end())
	{
		for (auto& material : m_shader_dependencies[p_hash].materials)
			material->invalidate();

		for (auto& pipeline : m_shader_dependencies[p_hash].pipelines)
			pipeline->invalidate();
	}
}

uint32_t Renderer::get_current_frame_index()
{
    constexpr auto backend = get_render_backend_type();
    switch (backend)
    {
    case backend::render_backend_type_t::vulkan:
        return backend::vk::vulkan_context::get()->get_swap_chain()->GetCurrentBufferIndex();
    default:
    {
        KB_CORE_ASSERT(false, "Unhandled render backend type!");
        return 0;
    }
    }
}

void Renderer::wait_and_render(render_thread* rendering_thread)
{
    KB_PROFILE_SCOPE;
	KB_CORE_ASSERT(rendering_thread, "render thread is null?");

    auto& thread_performance_timers = Application::Get().get_thread_performance_timings_mut();

	{
        const timer render_thread_wait_timer{};
		rendering_thread->wait_and_set(thread_state_t::kick, thread_state_t::busy);
        thread_performance_timers.render_thread_wait_time = render_thread_wait_timer.get_elapsed_ms();
	}

	// execute command queue
    const timer render_thread_work_timer{};
	m_command_queues[get_render_command_queue_index()].execute();
	rendering_thread->set(thread_state_t::idle);
    thread_performance_timers.render_thread_work_time = render_thread_work_timer.get_elapsed_ms();
}

// main render function which runs on render thread
void Renderer::render_thread_func(render_thread* rendering_thread)
{
    KB_PROFILE_SCOPE;
	while (rendering_thread->is_running())
		wait_and_render(rendering_thread);
}

void Renderer::swap_queues()
{
	m_render_command_queue_submission_index = (m_render_command_queue_submission_index + 1) % s_render_command_queue_size;
}
} // end namespace kb::render
