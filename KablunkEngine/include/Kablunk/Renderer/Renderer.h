#ifndef KABLUNK_RENDERER_RENDERER_H
#define KABLUNK_RENDERER_RENDERER_H

#include "Kablunk/Core/Singleton.h"

#include "Kablunk/Renderer/RendererTypes.h"
#include "Kablunk/Renderer/OrthographicCamera.h"
#include "Kablunk/Renderer/Shader.h"
#include "Kablunk/Renderer/Texture.h"
#include "Kablunk/Renderer/Mesh.h"
#include "Kablunk/Renderer/EditorCamera.h"
#include "Kablunk/Renderer/UniformBuffer.h"
#include "Kablunk/Renderer/Pipeline.h"
#include "Kablunk/Renderer/UniformBufferSet.h"
#include "Kablunk/Renderer/Material.h"
#include "Kablunk/Renderer/RendererAPI.h"
#include "Kablunk/Core/RenderThread.h"
#include "Kablunk/Renderer/render_command_queue.h"
#include "Kablunk/Renderer/compute_pipeline.h"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace kb
{
// forward declaration
class RendererAPI;

constexpr uint32_t MAX_POINT_LIGHTS = 16;

enum RendererPipelineDescriptor
{
	PHONG_DIFFUSE = 0,
	PBR = 1,

	NONE
};

struct RendererOptions
{
	uint32_t frames_in_flight = 3;
	RendererPipelineDescriptor pipeline = PHONG_DIFFUSE;
};

// #TODO refactor rendererapi (remove) by moving into renderer
class Renderer
{
public:
	// typedef for main render thread function
	using render_thread_func_t = void(*)(Renderer*, render_thread*);
public:
	void init();
	void shutdown();

	void RegisterShaderDependency(ref<Shader> shader, ref<Pipeline> pipeline);
	void register_shader_dependency(ref<Shader> p_shader, ref<kb::render::compute_pipeline> p_compute_pipeline);
	void RegisterShaderDependency(ref<Shader> shader, ref<Material> material);
	void OnShaderReloaded(uint64_t hash);

	uint32_t GetCurrentFrameIndex();

	ref<ShaderLibrary> GetShaderLibrary();
	ref<Shader> GetShader(const std::string& name);

	const RendererOptions& GetConfig() { return m_options; }

	const RendererPipelineDescriptor GetRendererPipeline() { return m_options.pipeline; }
	void SetRendererPipeline(RendererPipelineDescriptor new_pipeline) { m_options.pipeline = new_pipeline; }

	static RendererAPI::render_api_t GetAPI() { return RendererAPI::GetAPI(); };

	// \brief get the viewport's os screen position within the application
	const glm::vec2& get_viewport_pos() const { return m_viewport_pos; }
	// \brief get the viewport's size
	const glm::vec2& get_viewport_size() const { return m_viewport_size; }

	// #TODO remove when rendererapi is refactored
	RendererAPI* get_renderer() { KB_CORE_ASSERT(m_renderer_api, "RendererAPI not set?"); return m_renderer_api; }

	// ==============
	// multithreading
	// ==============

	// wait for frame data to finish rendering
	void wait_and_render(render_thread* render_thread);
	// main render function which runs on render thread
	void render_thread_func(render_thread* rendering_thread);
	// swap rendering command queues
	void swap_queues();
	// get the current render queue index
	u32 get_render_command_queue_index() const { return (m_render_command_queue_submission_index + 1) % s_render_command_queue_size; }
	// get the current render queue submission index
	u32 get_render_command_queue_submission_index() const { return m_render_command_queue_submission_index; }
	// get a mutable reference to a render command queue
	kb::render_command_queue& get_render_command_queue() { return m_command_queues[m_render_command_queue_submission_index]; }
	// get a mutable reference to a resource release queue
	kb::render_command_queue& get_resource_free_queue(size_t index) { KB_CORE_ASSERT(index < s_resource_free_queue_size, "index out of bounds!"); return m_resource_free_queue[index]; }

	SINGLETON_GET_FUNC(Renderer);
private:
	struct ShaderDependencies
	{
		std::vector<ref<Pipeline>> pipelines;
		std::vector<ref<Material>> materials;
        std::vector<ref<kb::render::compute_pipeline>> compute_pipelines;
	};

	kb::unordered_flat_map<uint64_t, ShaderDependencies> m_shader_dependencies;
	RendererOptions m_options = { };
	ref<ShaderLibrary> m_shader_library;
	RendererAPI* m_renderer_api = nullptr;

	// store the viewport's os screen position within the application
	// used for calculating screen to world space in the editor
	glm::vec2 m_viewport_pos = glm::vec2{ 0.0f };
	// store the viewport's size
	// used for calculating screen to world space in the editor
	glm::vec2 m_viewport_size = glm::vec2{ 0.0f };
	// submission index of render command queue
	std::atomic<u32> m_render_command_queue_submission_index = 0;
	// number of render command queues
	constexpr static const u32 s_render_command_queue_size = 2;
	constexpr static const u32 s_resource_free_queue_size = 3;

	// resource freeing queues
	kb::render_command_queue m_resource_free_queue[s_resource_free_queue_size]{};
	// render command queues
	kb::render_command_queue m_command_queues[s_render_command_queue_size];

	friend class EditorLayer;
};
}

#endif
