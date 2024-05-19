#ifndef KABLUNK_RENDERER_RENDERER_H
#define KABLUNK_RENDERER_RENDERER_H

#include "Kablunk/Core/Singleton.h"

#include "Kablunk/Renderer/Shader.h"
#include "Kablunk/Renderer/Texture.h"
#include "Kablunk/Renderer/Mesh.h"
#include "Kablunk/Renderer/Pipeline.h"
#include "Kablunk/Renderer/Material.h"
#include "Kablunk/Renderer/RendererAPI.h"
#include "Kablunk/Core/RenderThread.h"
#include "Kablunk/Renderer/render_command_queue.h"
#include "Kablunk/Renderer/compute_pipeline.h"
#include "Kablunk/Renderer/render_backend.h"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Platform/Vulkan/vulkan_render_backend.h"

namespace kb
{ // start namespace kb
// forward declaration
class EditorLayer;
} // end namespace kb

namespace kb::render
{ // start namespace kb::render

constexpr uint32_t MAX_POINT_LIGHTS = 16;

struct RendererOptions
{
    uint32_t frames_in_flight = 3;
};

// #TODO refactor rendererapi (remove) by moving into renderer
class Renderer
{
public:
	// type alias for main render thread function
	using render_thread_func_t = void(*)(Renderer*, render_thread*);

    // #TODO expose compile time backend switch
    inline static constexpr render_backend_type_t k_render_backend_type = render_backend_type_t::vulkan;
    using render_backend_t = render_backend<vulkan_render_backend>;
public:
	void init();
	void shutdown();

	void RegisterShaderDependency(ref<Shader> shader, ref<Pipeline> pipeline);
	void register_shader_dependency(ref<Shader> p_shader, ref<compute_pipeline> p_compute_pipeline);
	void RegisterShaderDependency(ref<Shader> shader, ref<Material> material);
	void OnShaderReloaded(uint64_t hash);

	uint32_t GetCurrentFrameIndex();

	ref<ShaderLibrary> GetShaderLibrary();
	ref<Shader> GetShader(const std::string& name);

	const RendererOptions& get_config() const noexcept { return m_options; }

	// \brief get the viewport's os screen position within the application
	const glm::vec2& get_viewport_pos() const { return m_viewport_pos; }
	// \brief get the viewport's size
	const glm::vec2& get_viewport_size() const { return m_viewport_size; }

    static constexpr auto get_render_backend_type() noexcept -> render_backend_type_t
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
	render_command_queue& get_render_command_queue()
	{
	    return m_command_queues[m_render_command_queue_submission_index];
	}

	// get a mutable reference to a resource release queue
	render_command_queue& get_resource_free_queue(size_t index)
	{
	    KB_CORE_ASSERT(index < s_resource_free_queue_size, "index out of bounds!");
	    return m_resource_free_queue[index];
	}

	SINGLETON_GET_FUNC(Renderer);
private:
	struct ShaderDependencies
	{
		std::vector<ref<Pipeline>> pipelines;
		std::vector<ref<Material>> materials;
        std::vector<ref<compute_pipeline>> compute_pipelines;
	};

	kb::unordered_flat_map<uint64_t, ShaderDependencies> m_shader_dependencies;
	RendererOptions m_options = { };
	ref<ShaderLibrary> m_shader_library;
    // #TODO expose changing render backend at compile time...
    render_backend_t m_backend{};

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
	render_command_queue m_resource_free_queue[s_resource_free_queue_size]{};
	// render command queues
	render_command_queue m_command_queues[s_render_command_queue_size];

	friend class ::kb::EditorLayer;
};
} // end namespace kb::render

#endif
