#pragma once
#ifndef KABLUNK_RENDERER_RENDERER_2D
#define KABLUNK_RENDERER_RENDERER_2D

#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include "Kablunk/Core/RefCounting.h"

#include "Kablunk/Asset/AssetManager.h"

#include "Kablunk/Renderer/Texture.h"
#include "Kablunk/Renderer/OrthographicCamera.h"
#include "Kablunk/Renderer/EditorCamera.h"
#include "Kablunk/Renderer/Camera.h"
#include "Kablunk/Scene/Entity.h"

#include "Kablunk/Renderer/SceneRenderer.h"

#include "Kablunk/Renderer/UniformBuffer.h"
#include "Kablunk/Renderer/Shader.h"
#include "Kablunk/Renderer/Buffer.h"

#include "Kablunk/Renderer/Font/FontManager.h"

namespace kb
{ // start namespace kb

// forward declaration
namespace asset
{ // start namespace ::asset

} // end namespace ::asset 

struct QuadVertex
{
	glm::vec3 Position;
	glm::vec4 Color;
	glm::vec2 TexCoord;
	float TexIndex;
	float TilingFactor;
    //u8 padding[20]{ 0 };
};

// #TODO remove when entity id is removed from quadvertex
struct UIQuadVertex
{
	glm::vec3 Position;
	glm::vec4 Color;
	glm::vec2 TexCoord;
	float TexIndex;
	float TilingFactor;
};

struct CircleVertex
{
	glm::vec3 WorldPosition;
	glm::vec3 LocalPosition;
	glm::vec4 Color;
	float Radius;
	float Thickness;
	float Fade;

	// #TODO figure out how to pass 64 bit integers to OpenGL
	int32_t EntityID{ -1 };
};

struct LineVertex
{
	glm::vec3 Position;
	glm::vec4 Color;
};

struct text_vertex_t
{
	glm::vec3 m_position;
	glm::vec4 m_tint_color;
	glm::vec2 m_tex_coord;
	float m_tex_index;

    //u8 padding[24]{ 0 };
};

struct renderer_2d_specification_t
{
	bool swap_chain_target = false;
};

struct renderer_2d_stats_t
{
	uint32_t Draw_calls = 0;
	uint32_t Quad_count = 0;
	uint32_t Circle_count = 0;
	uint32_t batch_count = 0;

	uint32_t GetTotalVertexCount() { return Quad_count * 4; }
	uint32_t GetTotalIndexCount() { return Quad_count * 6; }
};


// #TODO move onto renderer class, this is leftover from old static renderer
struct renderer_2d_data_t
{
	static const uint32_t max_quads = 20'000;
	static const uint32_t max_vertices = max_quads * 4;
	static const uint32_t max_indices = max_quads * 6;
	static constexpr const uint32_t max_lines = 10000;
	static constexpr const uint32_t max_line_vertices = max_lines * 2;
	static constexpr const uint32_t max_line_indices = max_lines * 6;
	static const uint32_t max_texture_slots = 32;
	glm::vec4 quad_vertex_positions[4] = {};

	// quads
    using vertex_per_frame_buffer = std::vector<ref<VertexBuffer>>;
	std::vector<vertex_per_frame_buffer> quad_vertex_buffers;
    ref<IndexBuffer> quad_index_buffer;

	// circle
	std::vector<vertex_per_frame_buffer> circle_vertex_buffers;

	// lines
	std::vector<vertex_per_frame_buffer> line_vertex_buffers;
    ref<IndexBuffer> line_index_buffer;

	// ui quads
	std::vector<vertex_per_frame_buffer> ui_quad_vertex_buffers;
    ref<IndexBuffer> ui_quad_index_buffer;

	// text
	std::vector<vertex_per_frame_buffer> text_vertex_buffers;
    ref<IndexBuffer> text_index_buffer;

	// =======
	// shaders
	// =======

    ref<Shader> quad_shader;
    ref<Shader> circle_shader;
    ref<Shader> line_shader;
    ref<Shader> ui_shader;
	ref<Shader> text_shader;

	// =======

    ref<Texture2D> white_texture;

	// Quads
    // base pointers act as a rudimentary, manually controlled ring buffer
    using quad_per_frame_base_buffer = std::vector<QuadVertex*>;
	std::vector<quad_per_frame_base_buffer> quad_vertex_buffer_base_ptrs;
	std::vector<QuadVertex*> quad_vertex_buffer_ptrs;
	uint32_t quad_count = 0;
	uint32_t quad_index_count = 0;
    u32 m_quad_write_index = 0;

	// Circles
    using circle_per_frame_base_buffer = std::vector<CircleVertex*>;
	std::vector<circle_per_frame_base_buffer> circle_vertex_buffer_base_ptrs;
	std::vector<CircleVertex*> circle_vertex_buffer_ptr;
	uint32_t circle_count = 0;
	uint32_t circle_index_count = 0;
    u32 m_circle_write_index = 0;

	// Lines
    using line_per_frame_base_buffer = std::vector<LineVertex*>;
	std::vector<line_per_frame_base_buffer> line_vertex_buffer_base_ptrs;
	std::vector<LineVertex*> line_vertex_buffer_ptr;
	uint32_t line_count = 0;
	uint32_t line_index_count = 0;
    u32 m_line_write_index = 0;
	float line_width = 1.0f;

	// text
    using text_per_frame_base_buffer = std::vector<text_vertex_t*>;
	std::vector<text_per_frame_base_buffer> text_vertex_buffer_base_ptrs;
    std::vector<text_vertex_t*> text_vertex_buffer_ptr;
	uint32_t text_count = 0;
	uint32_t text_index_count = 0;
    u32 m_text_write_index = 0;

	uint32_t texture_slot_index = 1; //0 = white texture
	u32 text_texture_atlas_slot_index = 0;

	// TODO: change to asset handle when implemented
	std::array<ref<Texture2D>, max_texture_slots> texture_slots;
	std::array<ref<Texture2D>, max_texture_slots> text_texture_atlas_slots;

	ref<RenderCommandBuffer> render_command_buffer;

	render::font_manager m_font_manager;

	ref<Pipeline> quad_pipeline;
	ref<Pipeline> circle_pipeline;
    ref<Pipeline> line_pipeline;
    ref<Pipeline> ui_pipeline;
	ref<Pipeline> text_pipeline;

    ref<Material> quad_material;
    ref<Material> circle_material;
    ref<Material> line_material;
    ref<Material> ui_material;
	ref<Material> text_material;

    ref<UniformBufferSet> uniform_buffer_set;

	renderer_2d_specification_t specification;

	renderer_2d_stats_t Stats = {};

	struct GPUQueryTimeIndex
	{
		uint64_t renderer_2D_query;
	} gpu_time_query;
};

class Renderer2D : public RefCounted
{
public:
	Renderer2D() = default;
	~Renderer2D();
		
	void init(renderer_2d_specification_t spec = {});
	void shutdown();

    auto set_asset_manager(ref<asset::AssetManager> p_asset_manager) -> void;

    ref<Texture2D> get_white_texture();

	void begin_scene(const Camera& camera, const glm::mat4& transform, bool p_explicit_clear = false);
	void begin_scene(const EditorCamera& camera, bool p_explicit_clear = false);
    void begin_scene(const glm::mat4& p_projection, const glm::mat4& p_transform, bool p_explicit_clear = false);
	void end_scene();
	void flush();

	void on_imgui_render() const;

    ref<RenderPass> get_target_render_pass();
	void set_target_render_pass(ref<RenderPass> render_pass);
	void on_recreate_swapchain();
    void on_viewport_resize(const glm::vec2& p_viewport_dimensions);

    void set_swap_chain_target(bool p_swap_chain_target = true);

	// return a mutable reference to the font manager
	render::font_manager& get_font_manager() { return m_renderer_data.m_font_manager; }
	// return an immutable reference to the font manager
	const render::font_manager& get_font_manager() const { return m_renderer_data.m_font_manager; }

    // ---draw commands-------------------------------------------------------------------------------------------------

	// Entity
	void draw_sprite(Entity entity);

	// draw quad
	void draw_quad(
		const glm::vec2& position, 
		const glm::vec2& size, 
		const ref<Texture2D>& texture, 
		float tilingFactor = 1.0f, 
		const glm::vec4& tintColor = glm::vec4{ 1.0f }
	);
	// draw quad
	void draw_quad(
		const glm::vec3& position,
		const glm::vec2& size, 
		const ref<Texture2D>& texture, 
		float tilingFactor = 1.0f, 
		const glm::vec4& tintColor = glm::vec4{ 1.0f }
	);
	// draw quad
	// #TODO figure out how to pass 64 bit integers to OpenGL so we can support int64_t instead of int32_t
	void draw_quad(
		const glm::mat4& transform, 
		const ref<Texture2D>& texture, 
		float tilingFactor = 1.0f, 
		const glm::vec4& tintColor = glm::vec4{ 1.0f }, 
		int32_t entity_id = -1
	);

	// draw quad from texture atlas
	void draw_quad_from_texture_atlas(
		const glm::vec2& position, 
		const glm::vec2& size, 
		const ref<Texture2D>& texture, 
		const glm::vec2* texture_atlas_offsets, 
		float tiling_factor = 1.0f, 
		const glm::vec4& tint_color = glm::vec4{ 1.0f }
	);
	// draw quad from texture atlas
	void draw_quad_from_texture_atlas(
		const glm::vec3& position, 
		const glm::vec2& size, 
		const ref<Texture2D>& texture, 
		const glm::vec2* texture_atlas_offsets, 
		float tiling_factor = 1.0f, 
		const glm::vec4& tint_color = glm::vec4{ 1.0f }
	);
	// draw quad from texture atlas
	void draw_quad_from_texture_atlas(
		const glm::mat4& transform,
		const glm::vec2& size, 
		const ref<Texture2D>& texture, 
		const glm::vec2* texture_atlas_offsets, 
		float tiling_factor = 1.0f, 
		const glm::vec4& tint_color = glm::vec4{ 1.0f }
	);

	// draw circle
	void draw_circle(
		const glm::mat4& transform, 
		const glm::vec4& color, 
		float radius = 0.5f, 
		float thickness = 1.0f, 
		float fade = 0.005f, 
		int32_t entity_id = -1
	);

	// draw line
	void draw_line(
		const glm::vec3& p0, 
		const glm::vec3& p1, 
		const glm::vec4& color = glm::vec4{ 1.0f }
	);

	// draw rectangle
	void draw_rect(const glm::vec2& position, const glm::vec2& size, float rotation = 0, const glm::vec4& color = glm::vec4{ 1.0f });
	// draw rectangle
	void draw_rect(const glm::vec3& position, const glm::vec2& size, float rotation = 0, const glm::vec4& color = glm::vec4{ 1.0f });

	// draw text string
	void draw_text_string(
		const std::string& text,
		const glm::vec2& position, 
		const glm::vec2& size, 
		const ref<render::font_asset_t>& font_asset, 
		const glm::vec4& tint_color = glm::vec4{ 1.0f }
	);
	// draw text string 
	void draw_text_string(
		const std::string& text, 
		const glm::vec3& position, 
		const glm::vec2& size, 
		const ref<render::font_asset_t>& font_asset, 
		const glm::vec4& tint_color = glm::vec4{ 1.0f }
	);

    // -----------------------------------------------------------------------------------------------------------------

    // add a texture which can be rendered during the current scene
    // returns texture index (float) of slot occupied
    auto add_texture(const ref<Texture2D>& p_texture) -> f32;

    // submit batched quad data
    auto submit_quad_data(const Buffer& p_quad_buffer) -> void;

	// reset renderer2d stats
	void reset_stats();
	// get renderer2d stats
	renderer_2d_stats_t get_stats();
private:
	void start_new_batch();
	void end_batch();

    auto get_writeable_quad_buffer(u32 p_new_quad_count = 0) -> QuadVertex*&;
    auto get_writeable_circle_buffer(u32 p_new_circle_count = 0) -> CircleVertex*&;
    auto get_writeable_line_buffer(u32 p_new_line_count = 0) -> LineVertex*&;
    auto get_writeable_text_buffer(u32 p_new_text_count = 0) -> text_vertex_t*&;

    auto add_quad_buffer() -> void;
    auto add_circle_buffer() -> void;
    auto add_line_buffer() -> void;
    auto add_text_buffer() -> void;
private:
    renderer_2d_data_t m_renderer_data{};
    ref<asset::AssetManager> m_asset_manager = nullptr;
    bool m_explicit_render_pass_clear = false;
};

} // end namespace Kablunk

#endif
