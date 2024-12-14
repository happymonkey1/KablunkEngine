#include "kablunkpch.h"
#include "Kablunk/Renderer/Renderer.h"


#include "Kablunk/Core/Application.h"
#include "Kablunk/Core/Timers.h"

#define ENABLE_PBR_RENDERER 0

namespace kb::render
{ // start namespace kb::render
void Renderer::init()
{
    KB_PROFILE_SCOPE;

#if !ENABLE_PBR_RENDERER
    if (m_options.m_renderer_pipeline_type == renderer_pipeline_type_t::pbr)
    {
        KB_CORE_WARN("[renderer]: PBR renderer pipeline is not finished, defaulting to basic pipeline instead!");
        m_options.m_renderer_pipeline_type = renderer_pipeline_type_t::basic;
    }
#endif

	// initialize render command queues
	for (size_t i = 0; i < k_render_command_queue_size; ++i)
		m_command_queues[i] = backend::render_command_queue{};

    // Initialize graphics context
    m_context = backend::graphics_context::create(nullptr);

	m_shader_library = arc<shader_library>::Create();

	// ==========
	// 3d shaders
	// ==========

    switch (m_options.m_renderer_pipeline_type)
    {
    case renderer_pipeline_type_t::pbr:
    {
        KB_CORE_INFO("[renderer]: Loading PBR pipeline renderer shaders");

        m_shader_library->load(fmt::format(
            "resources/shaders/{}.glsl",
            shader_library::k_pbr_static_shader_name
        ));

        break;
    }
    case renderer_pipeline_type_t::basic:
    {
        KB_CORE_INFO("[renderer]: Loading basic pipeline renderer shaders");

        m_shader_library->load(fmt::format(
            "resources/shaders/{}.glsl",
            shader_library::k_diffuse_static_shader_name
        ));

        break;
    }
    default:
        KB_CORE_ASSERT(false, "[renderer]: Cannot load shaders for unknown renderer pipeline type!");
    }
    
    m_shader_library->load(fmt::format(
        "resources/shaders/{}.glsl",
        shader_library::k_scene_composite_name
    ));

	// ==========
	// 2d shaders
	// ==========

    KB_CORE_INFO("[renderer]: Loading 2D shaders");
    m_shader_library->load(fmt::format(
        "resources/shaders/{}.glsl",
        shader_library::k_renderer_2d_quad_name
    ));
    m_shader_library->load(fmt::format(
        "resources/shaders/{}.glsl",
        shader_library::k_renderer_2d_circle_name
    ));
    m_shader_library->load(fmt::format(
        "resources/shaders/{}.glsl",
        shader_library::k_renderer_2d_line_name
    ));
    m_shader_library->load(fmt::format(
        "resources/shaders/{}.glsl",
        shader_library::k_renderer_2d_UI_name
    ));
    m_shader_library->load(fmt::format(
        "resources/shaders/{}.glsl",
        shader_library::k_renderer_2d_text_name
    ));

	// ==========

    // Load renderer's white texture
    constexpr u32 white_texture_data = 0xFFFFFFFF;
    m_white_texture = backend::texture_2d::create(
        backend::image_format_t::RGBA,
        1,
        1,
        &white_texture_data
    );

    m_virtual_texture_registry = virtual_texture_registry::create();

    const auto& application = Application::Get();
    if (application.get_render_thread().is_running())
    {
        // compile shaders that were submitted
        Application::Get().get_render_thread().pump();
    }
    else
    {
        KB_CORE_WARN("[renderer]: Shaders have not been compiled since render thread is not running!");
    }

    // Initialize rendering backend
    m_backend = backend::render_backend::create(m_backend_type, m_context.get());
    m_backend->init();
}

void Renderer::shutdown()
{
    KB_PROFILE_SCOPE;

	m_shader_dependencies.clear();

	m_shader_library.reset();

	// render2d::shutdown();

    delete m_backend;
    m_backend = nullptr;

	for (size_t i = 0; i < k_render_command_queue_size; ++i)
        if (!m_command_queues[i].is_empty())
			KB_CORE_WARN("[renderer]: renderer shutting down but command_queue[{}] is not empty?", i);

    m_context->destroy();
}

const arc<shader_library>& Renderer::get_shader_library()
{
	return m_shader_library;
}

const arc<backend::shader>& Renderer::get_shader(const std::string& p_shader_name)
{
	return m_shader_library->get(p_shader_name);
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

uint32_t Renderer::get_current_frame_index() const noexcept
{
    switch (Singleton<Renderer>::get().get_render_backend_type())
    {
    case backend::render_backend_type_t::vulkan:
        return m_context->get_swap_chain()->get_current_buffer_index();
    case backend::render_backend_type_t::none:
        return Application::Get().get_current_frame_index();
    default:
    {
        KB_CORE_ASSERT(false, "Unhandled render backend type!");
        return 0;
    }
    }
}

auto Renderer::create_texture(
    const std::filesystem::path& p_filepath
) const noexcept -> virtual_texture_handle
{
    return m_virtual_texture_registry->load_individual_texture(p_filepath);
}

auto Renderer::create_texture(
    backend::texture_specification_t p_specification,
    const void* p_data
)  const noexcept -> virtual_texture_handle
{
    KB_CORE_ASSERT(false, "not implemented!");
    return {};
}

auto Renderer::get_texture(virtual_texture_handle p_handle) const noexcept -> const arc<backend::texture_2d>&
{
    return m_virtual_texture_registry->get_texture_2d_by_virtual_handle(p_handle);
}

void Renderer::wait_and_render(render_thread* p_rendering_thread)
{
    KB_PROFILE_SCOPE;
	KB_CORE_ASSERT(p_rendering_thread, "render thread is null?");

    auto& thread_performance_timers = Application::Get().get_thread_performance_timings_mut();

	{
        const timer render_thread_wait_timer{};
		p_rendering_thread->wait_and_set(thread_state_t::kick, thread_state_t::busy);
        thread_performance_timers.render_thread_wait_time = render_thread_wait_timer.get_elapsed_ms();
	}

	// execute command queue
    const timer render_thread_work_timer{};
	m_command_queues[get_render_command_queue_index()].execute();
	p_rendering_thread->set(thread_state_t::idle);
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
	m_render_command_queue_submission_index = (m_render_command_queue_submission_index + 1) % k_render_command_queue_size;
}
} // end namespace kb::render
