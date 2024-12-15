#pragma once

#include "Kablunk/renderer/backend/render_command_queue.h"
#include "Kablunk/renderer/Renderer.h"
#include "Kablunk/renderer/shader_library.h"
#include "Kablunk/renderer/backend/material.h"
#include "Kablunk/renderer/backend/pipeline.h"
#include "Kablunk/renderer/backend/uniform_buffer_set.h"
#include "Kablunk/renderer/backend/storage_buffer_set.h"
#include "Kablunk/renderer/Mesh.h"
#include "Kablunk/renderer/material_asset.h"
#include "Kablunk/renderer/backend/compute_pipeline.h"

#include <mutex>

namespace kb::render
{
// =======
// new api
// =======

namespace details
{ // start namespace ::details

// to be called on render thread
inline void render_thread_func(render_thread* rendering_thread) noexcept
{
	Singleton<Renderer>::get().render_thread_func(rendering_thread);
}

} // end namespace ::details

// \brief get the os screen position of the viewport within the application
// used for converting screen to world position when in the editor
inline const glm::vec2& get_viewport_pos() noexcept
{
	return Singleton<Renderer>::get().get_viewport_pos();
}

// \brief get the size of the viewport
// used for converting screen to world position when in the editor
inline const glm::vec2& get_viewport_size() noexcept
{
	return Singleton<Renderer>::get().get_viewport_size();
}

// initialize the renderer
inline void init() noexcept
{
	Singleton<Renderer>::get().init();
}

// shutdown the renderer
inline void shutdown() noexcept
{
	KB_CORE_INFO("Renderer shutdown called!");
	Singleton<Renderer>::get().shutdown();
}

// begin rendering frame
inline void begin_frame() noexcept
{
    auto& renderer = Singleton<Renderer>::get();
    renderer.get_render_backend()->begin_frame();
}

// end renderering frame
inline void end_frame() noexcept { Singleton<Renderer>::get().get_render_backend()->end_frame(); }

// begin render pass
inline void begin_render_pass(
    const arc<backend::render_command_buffer>& p_render_command_buffer,
    const arc<backend::render_pass>& p_render_pass,
    bool explicit_clear = false
) noexcept
{
    auto& renderer = Singleton<Renderer>::get();
	renderer.get_render_backend()->begin_render_pass(
        p_render_command_buffer,
        p_render_pass,
        explicit_clear
    );
}

inline void end_render_pass(const arc<backend::render_command_buffer>& p_render_command_buffer) noexcept
{
	Singleton<Renderer>::get().get_render_backend()->end_render_pass(p_render_command_buffer);
}

// return a reference to the shader library
inline arc<shader_library> get_shader_library() noexcept
{
	return Singleton<Renderer>::get().get_shader_library();
}

// get a specific shader by name
inline arc<backend::shader> get_shader(const std::string& name) noexcept
{
	return Singleton<Renderer>::get().get_shader(name);
}

inline void register_shader_dependency(arc<backend::shader> shader, arc<backend::material> material) noexcept
{
	Singleton<Renderer>::get().register_shader_dependency(shader, material);
}

inline void register_shader_dependency(arc<backend::shader> shader, arc<backend::pipeline> pipeline) noexcept
{
	Singleton<Renderer>::get().register_shader_dependency(shader, pipeline);
}

inline void register_shader_dependency(arc<backend::shader> p_shader, arc<backend::compute_pipeline> p_compute_pipeline) noexcept
{
    Singleton<Renderer>::get().register_shader_dependency(p_shader, p_compute_pipeline);
}

inline void on_shader_reloaded(uint64_t hash) noexcept
{
	Singleton<Renderer>::get().on_shader_reloaded(hash);
}

// run submitted commands
inline void wait_and_render(render_thread* rendering_thread) noexcept
{
	Singleton<Renderer>::get().wait_and_render(rendering_thread);
}

// swap rendering queues
inline void swap_queues() noexcept
{
	Singleton<Renderer>::get().swap_queues();
}

// Retrieve an immutable arc to a white texture
inline auto get_white_texture() noexcept
{
    return Singleton<Renderer>::get().get_white_texture();
}

// Retrieve current renderer pipeline state
inline auto get_renderer_pipeline_type() noexcept -> renderer_pipeline_type_t
{
    return Singleton<Renderer>::get().get_renderer_pipeline_type();
}

// Retrieve a texture from the renderer based on a virtual handle
inline auto get_texture_2d(virtual_texture_handle p_handle) noexcept -> const arc<backend::texture_2d>&
{
    return Singleton<Renderer>::get().get_texture_2d(p_handle);
}

// Create a texture and return a virtual handle
// Texture is stored in the renderer's texture registry
inline auto create_texture(
    std::string_view p_name,
    const backend::texture_specification_t& p_specification,
    const void* p_data,
    const bool p_is_atlas = false
) noexcept -> virtual_texture_handle
{
    return Singleton<Renderer>::get().create_texture(
        p_name,
        p_specification,
        p_data,
        p_is_atlas
    );
}

// Create a texture and return a virtual handle
// Texture is stored in the renderer's texture registry
inline auto create_texture(
    const std::filesystem::path& p_filepath
) noexcept -> virtual_texture_handle
{
    return Singleton<Renderer>::get().create_texture(p_filepath);
}

// ======
// Meshes
// ======

#if 0
// render mesh with a material table
inline void render_mesh(
	arc<RenderCommandBuffer> render_command_buffer, 
	arc<Pipeline> pipeline, 
	arc<UniformBufferSet> uniform_buffer_set, 
	arc<StorageBufferSet> storage_buffer_set, 
	arc<Mesh> mesh, 
	uint32_t submesh_index, 
	arc<MaterialTable> material_table, 
	arc<VertexBuffer> transform_buffer, 
	uint32_t transform_offset, 
	uint32_t instance_count
)
{
	Singleton<Renderer>::get().get_renderer()->RenderMesh(
		render_command_buffer, 
		pipeline, 
		uniform_buffer_set, 
		storage_buffer_set, 
		mesh, 
		submesh_index, 
		material_table, 
		transform_buffer, 
		transform_offset, 
		instance_count
	);
}

// render mesh using a specific material
inline void render_mesh_with_material(
	arc<RenderCommandBuffer> render_command_buffer, 
	arc<Pipeline> pipeline, 
	arc<UniformBufferSet> uniform_buffer_set, 
	arc<StorageBufferSet> storage_buffer_set, 
	arc<Mesh> mesh, 
	uint32_t submesh_index, 
	arc<Material> material, 
	arc<VertexBuffer> transform_buffer, 
	uint32_t transform_offset, 
	uint32_t instance_count, 
	owning_buffer additional_uniforms = owning_buffer() // default (nullptr) buffer
)
{
	Singleton<Renderer>::get().get_renderer()->RenderMeshWithMaterial(
		render_command_buffer, 
		pipeline, 
		uniform_buffer_set, 
		storage_buffer_set, 
		mesh, submesh_index, 
		material, 
		transform_buffer, 
		transform_offset, 
		instance_count, 
		additional_uniforms
	);
}

// render instanced submesh with a material table
inline void render_instanced_submesh(
	arc<RenderCommandBuffer> render_command_buffer,
	arc<Pipeline> pipeline,
	arc<UniformBufferSet> uniform_buffer_set,
	arc<StorageBufferSet> storage_buffer_set,
	arc<Mesh> mesh,
	uint32_t submesh_index,
	arc<MaterialTable> material_table,
	arc<VertexBuffer> transform_buffer,
	uint32_t transform_offset,
	uint32_t instance_count
)
{
	Singleton<Renderer>::get().get_renderer()->render_instanced_submesh(
		render_command_buffer,
		pipeline,
		uniform_buffer_set,
		storage_buffer_set,
		mesh,
		submesh_index,
		material_table,
		transform_buffer,
		transform_offset,
		instance_count
	);
}

// =====
// Quads
// =====

// submit a full screen quad to be rendererd with a material
inline void submit_fullscreen_quad(arc<RenderCommandBuffer> render_command_buffer, arc<Pipeline> pipeline, arc<UniformBufferSet> uniform_buffer_set, arc<Material> material)
{
	Singleton<Renderer>::get().get_renderer()->SubmitFullscreenQuad(
		render_command_buffer,
		pipeline,
		uniform_buffer_set,
        arc<StorageBufferSet>{}, // storage buffer set
		material
	);
}

// submit a fullscreen quad to be rendered with a material (includes storage buffer set)
inline void SubmitFullscreenQuad(
	arc<RenderCommandBuffer> render_command_buffer,
	arc<Pipeline> pipeline,
	arc<UniformBufferSet> uniform_buffer_set,
	arc<StorageBufferSet> storage_buffer_set,
	arc<Material> material
)
{
	Singleton<Renderer>::get().get_renderer()->SubmitFullscreenQuad(
		render_command_buffer, 
		pipeline, 
		uniform_buffer_set, 
		storage_buffer_set,
		material
	);
}

// render a quad using a specific material and transform
inline void render_quad(
	arc<RenderCommandBuffer> render_command_buffer, 
	arc<Pipeline> pipeline, 
	arc<UniformBufferSet> uniform_buffer_set, 
	arc<StorageBufferSet> storage_buffer_set, 
	arc<Material> material, 
	const glm::mat4& transform
)
{
	Singleton<Renderer>::get().get_renderer()->RenderQuad(
		render_command_buffer, 
		pipeline, 
		uniform_buffer_set, 
		storage_buffer_set, 
		material,
		transform
	);
}

// render raw geometry
inline void render_geometry(
	arc<RenderCommandBuffer> render_command_buffer,
	arc<Pipeline> pipeline,
	arc<UniformBufferSet> uniform_buffer_set,
	arc<StorageBufferSet> storage_buffer_set,
	arc<Material> material,
	arc<VertexBuffer> vertex_buffer,
	arc<IndexBuffer> index_buffer,
	const glm::mat4& transform,
	uint32_t index_count = 0
)
{
	Singleton<Renderer>::get().get_renderer()->RenderGeometry(
		render_command_buffer,
		pipeline,
		uniform_buffer_set,
		storage_buffer_set,
		material,
		vertex_buffer,
		index_buffer,
		transform,
		index_count
	);
}
#endif

// render raw geometry
inline void render_geometry(
    const arc<backend::render_command_buffer>& p_render_command_buffer,
    const arc<backend::pipeline>& p_pipeline,
    const arc<backend::material>& p_material,
    const arc<backend::vertex_buffer>& p_vertex_buffer,
    const arc<backend::index_buffer>& p_index_buffer,
    const glm::mat4& p_transform,
    uint32_t p_index_count = 0
) noexcept
{
    Singleton<Renderer>::get().get_render_backend()->render_geometry(
        p_render_command_buffer,
        p_pipeline,
        p_material,
        p_vertex_buffer,
        p_index_buffer,
        p_transform,
        p_index_count
    );
}

inline void submit_fullscreen_quad(
    const arc<backend::render_command_buffer>& p_render_command_buffer,
    const arc<backend::pipeline>& p_pipeline,
    const arc<backend::material>& p_material
) noexcept
{
    Singleton<Renderer>::get().get_render_backend()->submit_fullscreen_quad(
        p_render_command_buffer,
        p_pipeline,
        p_material
    );
}

// =======
// utility
// =======

inline void set_line_width(const arc<backend::render_command_buffer>& p_render_command_buffer, f32 p_line_width) noexcept
{
	Singleton<Renderer>::get().get_render_backend()->set_line_width(p_render_command_buffer, p_line_width);
}

// get the number of frames in flight that will be rendered
inline uint32_t get_frames_in_flight() noexcept
{
	return Singleton<Renderer>::get().get_config().frames_in_flight;
}

// get the current index into which frame is being rendered
u32 get_current_frame_index() noexcept;

// get current index into the frame being rendered by the swapchain (which owns the render thread)
u32 rt_get_current_frame_index() noexcept;

inline backend::render_command_queue& get_render_resource_release_queue(uint32_t index) noexcept
{
	return Singleton<Renderer>::get().get_resource_free_queue(index);
}

inline backend::render_command_queue& get_render_command_queue() noexcept
{
	return Singleton<Renderer>::get().get_render_command_queue();
}

// submit a function to be queued (and run) on the render 
inline void submit(auto func) noexcept
{
    using func_t = decltype(func);

	auto render_cmd = [](void* ptr)
	{
		auto p_func = static_cast<func_t*>(ptr);
		(*p_func)();

		p_func->~func_t();
	};

	auto storage_buffer = get_render_command_queue().allocate(render_cmd, sizeof(func));
	new (storage_buffer) func_t(std::forward<func_t>(func));
}

inline void submit_resource_free(auto func) noexcept
{
    using func_t = decltype(func);

	auto render_cmd = [](void* ptr)
	{
		auto p_func = static_cast<func_t*>(ptr);
		(*p_func)();

		p_func->~func_t();
	};

	render::submit([render_cmd, func]()
		{
			const uint32_t index = rt_get_current_frame_index();
			auto storage_buffer = get_render_resource_release_queue(index).allocate(render_cmd, sizeof(func));
			new (storage_buffer) func_t(std::forward<func_t>(static_cast<func_t>(func)));
		});
}

}
