#ifndef KABLUNK_RENDERER_RENDERER_H
#define KABLUNK_RENDERER_RENDERER_H

#include "Kablunk/Core/Singleton.h"
#include "Kablunk/Core/render_thread.h"

#include "Kablunk/renderer/backend/shader.h"
#include "Kablunk/renderer/shader_library.h"
#include "Kablunk/renderer/backend/texture.h"
#include "Kablunk/renderer/Mesh.h"
#include "Kablunk/renderer/backend/pipeline.h"
#include "Kablunk/renderer/backend/material.h"
#include "Kablunk/renderer/backend/render_command_queue.h"
#include "Kablunk/renderer/backend/compute_pipeline.h"
#include "Kablunk/renderer/backend/render_backend.h"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Kablunk/Renderer/backend/backend_type.h"
#include "Kablunk/renderer/backend/graphics_context.h"
#include "Kablunk/renderer/backend/vulkan/vulkan_render_backend.h"

namespace kb
{ // start namespace kb

// forward declaration
class EditorLayer;
} // end namespace kb

namespace kb::render
{ // start namespace kb::render

constexpr uint32_t MAX_POINT_LIGHTS = 16;

struct renderer_options_t
{
    uint32_t frames_in_flight = 3;
};

class Renderer
{
public:
	// type alias for main render thread function
	using render_thread_func_t = void(*)(Renderer*, render_thread*);

    // #TODO expose compile time backend switch
    inline static constexpr backend::render_backend_type_t k_render_backend_type = backend::render_backend_type_t::vulkan;
    using underlying_render_backend_t = backend::vk::vulkan_render_backend;
    using render_backend_t = backend::render_backend<underlying_render_backend_t>;
public:
	void init();
	void shutdown();

    // register a shader dependency to a pipeline
	void register_shader_dependency(arc<backend::shader> p_shader, arc<backend::pipeline> p_pipeline);
    // register a shader dependency to a compute pipeline
	void register_shader_dependency(arc<backend::shader> p_shader, arc<backend::compute_pipeline> p_compute_pipeline);
    // register a shader dependency to a material
	void register_shader_dependency(arc<backend::shader> p_shader, arc<backend::material> p_material);
	void on_shader_reloaded(uint64_t p_hash);

    uint32_t get_current_frame_index() const noexcept;

	arc<shader_library> GetShaderLibrary();
	arc<backend::shader> GetShader(const std::string& name);

	const renderer_options_t& get_config() const noexcept { return m_options; }

	// \brief get the viewport's os screen position within the application
	const glm::vec2& get_viewport_pos() const { return m_viewport_pos; }
	// \brief get the viewport's size
	const glm::vec2& get_viewport_size() const { return m_viewport_size; }

    static constexpr auto get_render_backend_type() noexcept -> backend::render_backend_type_t
	{
	    return k_render_backend_type;
	}

    auto get_render_backend() const noexcept -> const render_backend_t& { return m_backend; }
    auto get_render_backend() noexcept -> render_backend_t& { return m_backend; }

	// ==============
	// multithreading
	// ==============

	// wait for frame data to finish rendering
	void wait_and_render(render_thread* rendering_thread);
	// main render function which runs on render thread
	void render_thread_func(render_thread* rendering_thread);
	// swap rendering command queues
	void swap_queues();
	// get the current render queue index
	u32 get_render_command_queue_index() const { return (m_render_command_queue_submission_index + 1) % s_render_command_queue_size; }
	// get the current render queue submission index
	u32 get_render_command_queue_submission_index() const { return m_render_command_queue_submission_index; }

	// get a mutable reference to a render command queue
    backend::render_command_queue& get_render_command_queue()
	{
	    return m_command_queues[m_render_command_queue_submission_index];
	}

	// get a mutable reference to a resource release queue
    backend::render_command_queue& get_resource_free_queue(size_t index)
	{
	    KB_CORE_ASSERT(index < s_resource_free_queue_size, "index out of bounds!");
	    return m_resource_free_queue[index];
	}

    // Submit a function to the render command queue.
    // Execution is deferred to the Render thread in multithreaded contexts
    auto submit(auto p_func) noexcept -> void
	{
        using func_t = decltype(p_func);

        auto cmd = [](void* p_storage) -> void
            {
                auto func = static_cast<func_t*>(p_storage);
                (*func)();

                func->~func_t();
            };

        auto* storage_buffer = get_render_command_queue().allocate(cmd, sizeof(p_func));
        // allocate input function in the render command queue
        new (storage_buffer) func_t(std::forward<func_t>(static_cast<func_t&&>(p_func)));
	}

    // Submit a function to the render resource release queue
    // Execution is deferred to the Render thread in multithreaded contexts
    auto submit_resource_free(auto p_func) noexcept -> void
	{
        using func_t = decltype(p_func);

        auto render_cmd = [](void* p_storage)
            {
                auto func = static_cast<func_t*>(p_storage);
                (*func)();

                func->~func_t();
            };

        if (render_thread::is_current_thread_rt())
        {
            const u32 rt_index = get_current_frame_index();
            auto storage_buffer = get_resource_free_queue(rt_index).allocate(render_cmd, sizeof(p_func));
            new (storage_buffer) func_t(std::forward<func_t>(static_cast<func_t&&>(p_func)));
        }
        else
        {
            const u32 rt_index = get_current_frame_index();
            const auto& resource_free_queue = get_resource_free_queue(rt_index);
            submit([render_cmd, p_func, &resource_free_queue]
                {
                    auto* storage_buffer = resource_free_queue.allocate(render_cmd, sizeof(p_func));
                    new (storage_buffer) func_t(std::forward<func_t>(static_cast<func_t&&>(p_func)));
                });
        }
	}

	SINGLETON_GET_FUNC(Renderer);
private:
	struct shader_dependencies_t
	{
		std::vector<arc<backend::pipeline>> pipelines;
		std::vector<arc<backend::material>> materials;
        std::vector<arc<backend::compute_pipeline>> compute_pipelines;
	};

	unordered_flat_map<uint64_t, shader_dependencies_t> m_shader_dependencies;
	renderer_options_t m_options = { };
	arc<shader_library> m_shader_library;
    // #TODO expose changing render backend at compile time...
    render_backend_t m_backend{};

    arc<backend::graphics_context> m_context;

	// store the viewport's os screen position within the application
	// used for calculating screen to world space in the editor
	glm::vec2 m_viewport_pos = glm::vec2{ 0.0f };
	// store the viewport's size
	// used for calculating screen to world space in the editor
	glm::vec2 m_viewport_size = glm::vec2{ 0.0f };
	// submission index of render command queue
	std::atomic<u32> m_render_command_queue_submission_index = 0;
	// number of render command queues
	constexpr static u32 s_render_command_queue_size = 3;
	constexpr static u32 s_resource_free_queue_size = 3;

	// resource freeing queues
    backend::render_command_queue m_resource_free_queue[s_resource_free_queue_size]{};
	// render command queues
    backend::render_command_queue m_command_queues[s_render_command_queue_size];

	friend class ::kb::EditorLayer;
};

} // end namespace kb::render

#endif
