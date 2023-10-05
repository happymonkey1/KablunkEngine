#pragma once
#ifndef KABLUNK_RENDERER_RENDERER_2D
#define KABLUNK_RENDERER_RENDERER_2D

#include "Kablunk/Core/RefCounting.h"

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

class AssetManager;

} // end namespace ::asset 

struct QuadVertex
{
	glm::vec3 Position;
	glm::vec4 Color;
	glm::vec2 TexCoord;
	float TexIndex;
	float TilingFactor;
	// #TODO figure out how to pass 64 bit integers to OpenGL
	int32_t EntityID{ -1 };
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
	std::vector<ref<VertexBuffer>> quad_vertex_buffers;
    ref <IndexBuffer> quad_index_buffer;

	// circle
	std::vector<ref<VertexBuffer>> circle_vertex_buffers;

	// lines
	std::vector<ref<VertexBuffer>> line_vertex_buffers;
    ref<IndexBuffer> line_index_buffer;

	// ui quads
	std::vector<ref<VertexBuffer>> ui_quad_vertex_buffers;
    ref<IndexBuffer> ui_quad_index_buffer;

	// text
	std::vector<ref<VertexBuffer>> text_vertex_buffers;
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
	std::vector<QuadVertex*> quad_vertex_buffer_base_ptrs;
	QuadVertex* quad_vertex_buffer_ptr = nullptr;
	uint32_t quad_count = 0;
	uint32_t quad_index_count = 0;

	// Circles
    // base pointers act as a rudimentary, manually controlled ring buffer
	std::vector<CircleVertex*> circle_vertex_buffer_base_ptrs;
	CircleVertex* circle_vertex_buffer_ptr = nullptr;
	uint32_t circle_count = 0;
	uint32_t circle_index_count = 0;

	// Lines
    // base pointers act as a rudimentary, manually controlled ring buffer
	std::vector<LineVertex*> line_vertex_buffer_base_ptrs;
	LineVertex* line_vertex_buffer_ptr = nullptr;
	uint32_t line_count = 0;
	uint32_t line_index_count = 0;
	float line_width = 1.0f;

	// UI
    // base pointers act as a rudimentary, manually controlled ring buffer
	std::vector<UIQuadVertex*> ui_quad_vertex_buffer_base_ptrs;
	UIQuadVertex* ui_quad_vertex_buffer_ptr = nullptr;
	uint32_t ui_quad_count = 0;
	uint32_t ui_quad_index_count = 0;

	// text
    // base pointers act as a rudimentary, manually controlled ring buffer
	std::vector<text_vertex_t*> text_vertex_buffer_base_ptrs;
	text_vertex_t* text_vertex_buffer_ptr = nullptr;
	uint32_t text_count = 0;
	uint32_t text_index_count = 0;

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

	Camera camera;
	glm::mat4 camera_transform;
	glm::mat4 camera_view_projection;

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

    void set_swap_chain_target(bool p_swap_chain_target = true);

	// return a mutable reference to the font manager
	render::font_manager& get_font_manager() { return m_renderer_data->m_font_manager; }
	// return an immutable reference to the font manager
	const render::font_manager& get_font_manager() const { return m_renderer_data->m_font_manager; }

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

	// draw ui quad
	void draw_quad_ui(
		const glm::vec2& position, 
		const glm::vec2& size, 
		const ref<Texture2D>& texture,
		float tiling_factor = 1.0f, 
		const glm::vec4& tint_color = glm::vec4{ 1.0f }
	);
	// draw ui quad
	void draw_quad_ui(
		const glm::vec3& position, 
		const glm::vec2& size, 
		const ref<Texture2D>& texture, 
		float tiling_factor = 1.0f, 
		const glm::vec4& tint_color = glm::vec4{ 1.0f }
	);
	// draw ui quad
	void draw_quad_ui(
		const glm::mat4& transform, 
		const ref<Texture2D>& texture, 
		float tiling_factor = 1.0f, 
		const glm::vec4& tint_color = glm::vec4{ 1.0f }
	);

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

	// reset renderer2d stats
	void reset_stats();
	// get renderer2d stats
	renderer_2d_stats_t get_stats();
private:
	void start_new_batch();
	void end_batch();
private:
	renderer_2d_data_t* m_renderer_data = nullptr;
    ref<asset::AssetManager> m_asset_manager = nullptr;
    bool m_explicit_render_pass_clear = false;
};

} // end namespace Kablunk

#endif
