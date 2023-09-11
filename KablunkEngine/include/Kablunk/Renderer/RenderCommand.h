#pragma once
#include "Kablunk/Renderer/RendererAPI.h"
#include "Kablunk/Renderer/render_command_queue.h"
#include "Kablunk/Renderer/Renderer.h"
#include "Kablunk/Renderer/Material.h"
#include "Kablunk/Renderer/Pipeline.h"
#include "Kablunk/Renderer/UniformBufferSet.h"
#include "Kablunk/Renderer/StorageBufferSet.h"
#include "Kablunk/Renderer/Mesh.h"
#include "Kablunk/Renderer/MaterialAsset.h"
#include "Kablunk/Renderer/compute_pipeline.h"

#include <mutex>

namespace Kablunk::render
{
// =======
// new api
// =======

namespace details
{ // start namespace ::details
	
// to be called on render thread
inline void render_thread_func(render_thread* rendering_thread)
{
	Singleton<Renderer>::get().render_thread_func(rendering_thread);
}

} // end namespace ::details

// \brief get the os screen position of the viewport within the application
// used for converting screen to world position when in the editor
inline const glm::vec2& get_viewport_pos()
{
	return Singleton<Renderer>::get().get_viewport_pos();
}

// \brief get the size of the viewport
// used for converting screen to world position when in the editor
inline const glm::vec2& get_viewport_size()
{
	return Singleton<Renderer>::get().get_viewport_size();
}

// initialize the renderer
inline void init()
{
	Singleton<Renderer>::get().init();
}

// shutdown the renderer
inline void shutdown()
{
	KB_CORE_INFO("Renderer shutdown called!");
	Singleton<Renderer>::get().shutdown();
}

inline RendererAPI* get_renderer() {
	return Singleton<Renderer>::get().get_renderer();
}

// begin rendering frame
inline void begin_frame() { Singleton<Renderer>::get().get_renderer()->BeginFrame(); }

// end renderering frame
inline void end_frame() { Singleton<Renderer>::get().get_renderer()->EndFrame(); }

// begin render pass
inline void begin_render_pass(IntrusiveRef<RenderCommandBuffer> render_command_buffer, const IntrusiveRef<RenderPass>& render_pass, bool explicit_clear = false)
{
	Singleton<Renderer>::get().get_renderer()->BeginRenderPass(render_command_buffer, render_pass, explicit_clear);
}

inline void end_render_pass(IntrusiveRef<RenderCommandBuffer> render_command_buffer)
{
	Singleton<Renderer>::get().get_renderer()->EndRenderPass(render_command_buffer);
}

inline RendererPipelineDescriptor get_render_pipeline()
{
	return Singleton<Renderer>::get().GetRendererPipeline();
}

// return a reference to the shader library
inline IntrusiveRef<ShaderLibrary> get_shader_library()
{
	return Singleton<Renderer>::get().GetShaderLibrary();
}

// get a specific shader by name
inline IntrusiveRef<Shader> get_shader(const std::string& name)
{
	return Singleton<Renderer>::get().GetShader(name);
}

inline void register_shader_dependency(IntrusiveRef<Shader> shader, IntrusiveRef<Material> material)
{
	Singleton<Renderer>::get().RegisterShaderDependency(shader, material);
}

inline void register_shader_dependency(IntrusiveRef<Shader> shader, IntrusiveRef<Pipeline> pipeline)
{
	Singleton<Renderer>::get().RegisterShaderDependency(shader, pipeline);
}

inline void register_shader_dependency(ref<Shader> p_shader, ref<kb::render::compute_pipeline> p_compute_pipeline)
{
    Singleton<Renderer>::get().register_shader_dependency(p_shader, p_compute_pipeline);
}

inline void on_shader_reloaded(uint64_t hash)
{
	Singleton<Renderer>::get().OnShaderReloaded(hash);
}

// run submitted commands
inline void wait_and_render(render_thread* rendering_thread)
{
	Singleton<Renderer>::get().wait_and_render(rendering_thread);
}

// swap rendering queues
inline void swap_queues()
{
	Singleton<Renderer>::get().swap_queues();
}

// set viewport dimensions
inline void set_viewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
{
	Singleton<Renderer>::get().get_renderer()->SetViewport(x, y, width, height);
}

inline void on_window_resize(uint32_t width, uint32_t height)
{
	set_viewport(0, 0, width, height);
}

// set framebuffer clear color
// DEPRECATED!
inline void set_clear_color(const glm::vec4& color)
{
	KB_CORE_ASSERT(false, "deprecated");
	Singleton<Renderer>::get().get_renderer()->SetClearColor(color);
}

// set framebuffer to clear color
// DEPRECATED!
inline void clear()
{
	KB_CORE_ASSERT(false, "deprecated");
	Singleton<Renderer>::get().get_renderer()->Clear();
};

// clear image
inline void clear_image(IntrusiveRef<RenderCommandBuffer> command_buffer, IntrusiveRef<Image2D> image)
{
	Singleton<Renderer>::get().get_renderer()->ClearImage(command_buffer, image);
}

// draw indexed vertex array
// DEPRECATED!
inline void draw_indexed(const IntrusiveRef<VertexArray> vertexArray, uint32_t indexCount = 0)
{
	KB_CORE_ASSERT(false, "deprecated");
	Singleton<Renderer>::get().get_renderer()->DrawIndexed(vertexArray, indexCount);
};

// set wireframe mode
inline void set_wireframe_mode(bool draw_wireframe)
{
	KB_CORE_ASSERT(false, "not implemented (correctly)");
	Singleton<Renderer>::get().get_renderer()->SetWireframeMode(draw_wireframe);
}

// ======
// Meshes
// ======

// render mesh with a material table
inline void render_mesh(
	IntrusiveRef<RenderCommandBuffer> render_command_buffer, 
	IntrusiveRef<Pipeline> pipeline, 
	IntrusiveRef<UniformBufferSet> uniform_buffer_set, 
	IntrusiveRef<StorageBufferSet> storage_buffer_set, 
	IntrusiveRef<Mesh> mesh, 
	uint32_t submesh_index, 
	IntrusiveRef<MaterialTable> material_table, 
	IntrusiveRef<VertexBuffer> transform_buffer, 
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
	IntrusiveRef<RenderCommandBuffer> render_command_buffer, 
	IntrusiveRef<Pipeline> pipeline, 
	IntrusiveRef<UniformBufferSet> uniform_buffer_set, 
	IntrusiveRef<StorageBufferSet> storage_buffer_set, 
	IntrusiveRef<Mesh> mesh, 
	uint32_t submesh_index, 
	IntrusiveRef<Material> material, 
	IntrusiveRef<VertexBuffer> transform_buffer, 
	uint32_t transform_offset, 
	uint32_t instance_count, 
	Buffer additional_uniforms = Buffer() // default (nullptr) buffer
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
	IntrusiveRef<RenderCommandBuffer> render_command_buffer,
	IntrusiveRef<Pipeline> pipeline,
	IntrusiveRef<UniformBufferSet> uniform_buffer_set,
	IntrusiveRef<StorageBufferSet> storage_buffer_set,
	IntrusiveRef<Mesh> mesh,
	uint32_t submesh_index,
	IntrusiveRef<MaterialTable> material_table,
	IntrusiveRef<VertexBuffer> transform_buffer,
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
inline void submit_fullscreen_quad(IntrusiveRef<RenderCommandBuffer> render_command_buffer, IntrusiveRef<Pipeline> pipeline, IntrusiveRef<UniformBufferSet> uniform_buffer_set, IntrusiveRef<Material> material)
{
	Singleton<Renderer>::get().get_renderer()->SubmitFullscreenQuad(
		render_command_buffer, 
		pipeline, 
		uniform_buffer_set, 
		nullptr, // storage buffer set
		material
	);
}

// submit a fullscreen quad to be rendered with a material (includes storage buffer set)
inline void SubmitFullscreenQuad(
	IntrusiveRef<RenderCommandBuffer> render_command_buffer, 
	IntrusiveRef<Pipeline> pipeline, 
	IntrusiveRef<UniformBufferSet> uniform_buffer_set, 
	IntrusiveRef<StorageBufferSet> storage_buffer_set, 
	IntrusiveRef<Material> material
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
	IntrusiveRef<RenderCommandBuffer> render_command_buffer, 
	IntrusiveRef<Pipeline> pipeline, 
	IntrusiveRef<UniformBufferSet> uniform_buffer_set, 
	IntrusiveRef<StorageBuffer> storage_buffer_set, 
	IntrusiveRef<Material> material, 
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
	IntrusiveRef<RenderCommandBuffer> render_command_buffer,
	IntrusiveRef<Pipeline> pipeline,
	IntrusiveRef<UniformBufferSet> uniform_buffer_set,
	IntrusiveRef<StorageBufferSet> storage_buffer_set,
	IntrusiveRef<Material> material,
	IntrusiveRef<VertexBuffer> vertex_buffer,
	IntrusiveRef<IndexBuffer> index_buffer,
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

// =======
// utility
// =======

inline void set_line_width(IntrusiveRef<RenderCommandBuffer> render_command_buffer, float line_width)
{
	Singleton<Renderer>::get().get_renderer()->SetLineWidth(render_command_buffer, line_width);
}

// get the number of frames in flight that will be rendered
inline uint32_t get_frames_in_flights()
{
	return Singleton<Renderer>::get().GetConfig().frames_in_flight;
}

// get the current index into which frame is being rendered
uint32_t get_current_frame_index();

// get current index into the frame being rendered by the swapchain (which owns the render thread)
u32 rt_get_current_frame_index();

inline kb::render_command_queue& get_render_resource_release_queue(uint32_t index)
{
	return Singleton<Renderer>::get().get_resource_free_queue(index);
}
// #TODO this is vulkan only so we should figure out an api agnostic way of dealing with this
inline kb::render_command_queue& get_render_command_queue()
{
	return Singleton<Renderer>::get().get_render_command_queue();
}

// #TODO this is vulkan only so we should figure out an api agnostic way of dealing with this
// submit a function to be queued (and run) on the render thread
template <typename FuncT>
inline void submit(FuncT&& func)
{
	//static std::mutex s_submit_mutex;
	//std::lock_guard lock{ s_submit_mutex };
	auto render_cmd = [](void* ptr)
	{
		auto p_func = (FuncT*)ptr;
		(*p_func)();

		p_func->~FuncT();
	};

	auto storage_buffer = get_render_command_queue().allocate(render_cmd, sizeof(func));
	new (storage_buffer) FuncT(std::forward<FuncT>(func));
}

template <typename FuncT>
inline void submit_resource_free(FuncT&& func)
{
	auto render_cmd = [](void* ptr)
	{
		auto p_func = (FuncT*)ptr;
		(*p_func)();

		p_func->~FuncT();
	};

	render::submit([render_cmd, func]()
		{
			const uint32_t index = rt_get_current_frame_index();
			auto storage_buffer = get_render_resource_release_queue(index).allocate(render_cmd, sizeof(func));
			new (storage_buffer) FuncT(std::forward<FuncT>((FuncT&&)func));
		});
}

}
