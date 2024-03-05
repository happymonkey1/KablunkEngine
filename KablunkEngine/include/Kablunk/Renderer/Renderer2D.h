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

#include "Kablunk/Renderer/SceneRenderer.h"

#include "Kablunk/Renderer/Shader.h"
#include "Kablunk/Renderer/Buffer.h"

#include "Kablunk/Renderer/Font/FontManager.h"

#include "Kablunk/Math/vec.hpp"

#include <glm/gtc/matrix_transform.hpp>

namespace kb
{ // start namespace kb

// #TODO move to separate file
namespace render
{ // start namespace render

using renderer_2d_texture_id = size_t;
inline static constexpr renderer_2d_texture_id k_invalid_renderer2d_texture_id = 4096ull;

} // end namespace render

// #TODO portable pack
#pragma pack(push, 1)
struct QuadVertex
{
	vec3_packed Position;
	vec4_packed Color;
	vec2_packed TexCoord;
    f32 TexIndex;
	f32 TilingFactor;
    //u8 padding[20]{ 0 };
};
#pragma pack(pop)

static_assert(sizeof(QuadVertex) == 44);

struct CircleVertex
{
    vec3_packed WorldPosition;
    vec3_packed LocalPosition;
	vec4_packed Color;
	float Radius;
	float Thickness;
	float Fade;

	// #TODO figure out how to pass 64 bit integers to OpenGL
	int32_t EntityID{ -1 };
};

struct LineVertex
{
	vec3_packed Position;
	vec4_packed Color;
};

struct text_vertex_t
{
	vec3_packed m_position;
	vec4_packed m_tint_color;
	vec2_packed m_tex_coord;
	float m_tex_index;

    //u8 padding[24]{ 0 };
};

static_assert(sizeof(text_vertex_t) == 12 + 16 + 8 + 4);

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
	static constexpr uint32_t max_quads = 20'000;
	static constexpr uint32_t max_vertices = max_quads * 4;
	static constexpr uint32_t max_indices = max_quads * 6;
    static constexpr uint32_t max_lines = 10'000;
	static constexpr uint32_t max_line_vertices = max_lines * 2;
	static constexpr uint32_t max_line_indices = max_lines * 6;
	static constexpr uint32_t max_texture_slots = 32;
    static constexpr std::array<vec2_packed, 4> k_texture_coords = {
        vec2_packed{0.0f, 0.0f},
        vec2_packed{ 1.0f, 0.0f },
        vec2_packed{ 1.0f, 1.0f },
        vec2_packed{ 0.0f, 1.0f }
    };
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

    auto set_asset_manager(const ref<asset::AssetManager>& p_asset_manager) -> void;

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
	void draw_sprite(Entity entity) noexcept;

	// draw quad
	inline void draw_quad(
		const glm::vec2& position,
		const glm::vec2& size,
		const ref<Texture2D>& texture,
		float tiling_factor = 1.0f,
		const glm::vec4& tint_color = glm::vec4{ 1.0f }
	) noexcept
    {
        //KB_PROFILE_SCOPE;

        draw_quad({ position.x, position.y, 0.0f }, size, texture, tiling_factor, tint_color);
    }

	// draw quad
	inline void draw_quad(
		const glm::vec3& position,
		const glm::vec2& size,
		const ref<Texture2D>& texture,
		float tiling_factor = 1.0f,
		const glm::vec4& tint_color = glm::vec4{ 1.0f }
	) noexcept
    {
        //KB_PROFILE_SCOPE;

        const glm::mat4 transform = glm::translate(glm::mat4(1.0f), position)
            * glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });

        draw_quad(transform, texture, tiling_factor, tint_color);
    }

	// draw quad
	// #TODO figure out how to pass 64-bit integers to OpenGL, so we can support int64_t instead of int32_t
	void draw_quad(
		const glm::mat4& transform,
		const ref<Texture2D>& texture,
		float tilingFactor = 1.0f,
		const glm::vec4& tintColor = glm::vec4{ 1.0f },
		int32_t entity_id = -1
	) noexcept;

	// draw quad from texture atlas
	inline void draw_quad_from_texture_atlas(
		const glm::vec2& position,
		const glm::vec2& size,
		const ref<Texture2D>& texture,
        const std::array<glm::vec2, 4>& texture_atlas_offsets,
		float tiling_factor = 1.0f,
		const glm::vec4& tint_color = glm::vec4{ 1.0f }
	) noexcept
    {
        //KB_PROFILE_SCOPE;

        draw_quad_from_texture_atlas(glm::vec3{ position.x, position.y, 0.0f }, size, texture, texture_atlas_offsets, tiling_factor, tint_color);
    }

	// draw quad from texture atlas
	inline void draw_quad_from_texture_atlas(
		const glm::vec3& position,
		const glm::vec2& size,
		const ref<Texture2D>& texture,
        const std::array<glm::vec2, 4>& texture_atlas_offsets,
		float tiling_factor = 1.0f,
		const glm::vec4& tint_color = glm::vec4{ 1.0f }
	) noexcept
    {
        //KB_PROFILE_SCOPE;

        glm::mat4 transform;
        {
            //KB_PROFILE_SCOPE_NAMED("Compute transform");
            static const glm::mat4 m{ 1.0f };

            transform = glm::translate(m, position)
            //  * glm::rotate(m, 0.0f, { 0.0f, 0.0f, 1.0f }) #NOTE disabled for performance while rotation in api is not supported
                * glm::scale(m, { size.x, size.y, 1.0f });
        }

        draw_quad_from_texture_atlas(transform, texture, texture_atlas_offsets, tiling_factor, tint_color);
    }

	// draw quad from texture atlas
	void draw_quad_from_texture_atlas(
		const glm::mat4& transform,
		const ref<Texture2D>& texture,
        const std::array<glm::vec2, 4>& texture_atlas_offsets,
		float tiling_factor = 1.0f,
		const glm::vec4& tint_color = glm::vec4{ 1.0f }
	) noexcept;
    // draw quad from texture atlas without a transform
    void draw_quad_from_texture_atlas_no_mat(
        const glm::vec4& position,
        const glm::vec2& size,
        const ref<Texture2D>& texture,
        const std::array<glm::vec2, 4>& texture_atlas_offsets,
        float tiling_factor = 1.0f,
        const glm::vec4& tint_color = glm::vec4{ 1.0f }
    ) noexcept;

    // draw quad (new api)
    // #TODO clean up naming of new api, remove old...
    inline auto submit_quad(
        const glm::vec3& p_position,
        const glm::vec2& p_size,
        render::renderer_2d_texture_id p_texture_id,
        const std::array<vec2_packed, 4>& p_texture_coords = renderer_2d_data_t::k_texture_coords,
        const glm::vec4& p_tint_color = glm::vec4{ 1.0f },
        float p_tiling_factor = 1.0f
    ) noexcept -> void
    {
        glm::mat4 transform;
        {
            //KB_PROFILE_SCOPE_NAMED("Compute transform");
            static const glm::mat4 m{ 1.0f };

            transform = glm::translate(m, p_position)
                //  * glm::rotate(m, 0.0f, { 0.0f, 0.0f, 1.0f }) #NOTE disabled for performance while rotation in api is not supported
                * glm::scale(m, { p_size.x, p_size.y, 1.0f });
        }

        submit_quad(transform, p_texture_id, p_texture_coords, p_tint_color, p_tiling_factor);
    }

    inline auto submit_quad(
        const glm::mat4& p_transform,
        render::renderer_2d_texture_id p_texture_id,
        const std::array<vec2_packed, 4>& p_texture_coords,
        const glm::vec4& p_tint_color,
        float tiling_factor = 1.0f
    ) noexcept -> void
    {
        //KB_PROFILE_SCOPE;

        constexpr size_t quad_vertex_count = 4;
        auto& quad_vertex_buffer_ptr = get_writeable_quad_buffer(1);

        for (uint32_t i = 0; i < quad_vertex_count; ++i)
        {
            quad_vertex_buffer_ptr->Position = p_transform * m_renderer_data.quad_vertex_positions[i];
            quad_vertex_buffer_ptr->Color = p_tint_color;
            quad_vertex_buffer_ptr->TexCoord = p_texture_coords[i];
            quad_vertex_buffer_ptr->TexIndex = p_texture_id;
            quad_vertex_buffer_ptr->TilingFactor = tiling_factor;
            quad_vertex_buffer_ptr++;
        }

        m_renderer_data.quad_index_count += 6;
        m_renderer_data.quad_count++;

        m_renderer_data.Stats.Quad_count += 1;
    }

	// draw circle
	void draw_circle(
		const glm::mat4& transform,
		const glm::vec4& color,
		float radius = 0.5f,
		float thickness = 1.0f,
		float fade = 0.005f,
		int32_t entity_id = -1
	) noexcept;

	// draw line
	void draw_line(
		const glm::vec3& p0,
		const glm::vec3& p1,
		const glm::vec4& color = glm::vec4{ 1.0f }
	) noexcept;

	// draw rectangle
	void draw_rect(const glm::vec2& position, const glm::vec2& size, float rotation = 0, const glm::vec4& color = glm::vec4{ 1.0f }) noexcept;
	// draw rectangle
	void draw_rect(const glm::vec3& position, const glm::vec2& size, float rotation = 0, const glm::vec4& color = glm::vec4{ 1.0f }) noexcept;

	// draw text string
	void draw_text_string(
		const std::string& text,
        const u32 p_font_point,
		const glm::vec2& position,
		const glm::vec2& size,
		const ref<render::font_asset_t>& font_asset,
		const glm::vec4& tint_color = glm::vec4{ 1.0f }
	) noexcept;
	// draw text string 
	void draw_text_string(
		const std::string& text,
        const u32 p_font_point,
		const glm::vec3& position,
		const glm::vec2& size,
		const ref<render::font_asset_t>& font_asset,
		const glm::vec4& tint_color = glm::vec4{ 1.0f }
	) noexcept;

    // -----------------------------------------------------------------------------------------------------------------

    // add a texture which can be rendered during the current scene
    // returns texture index (float) of slot occupied
    [[nodiscard]] inline auto submit_texture(const ref<Texture2D>& p_texture) noexcept -> render::renderer_2d_texture_id
    {
        //KB_PROFILE_SCOPE;

        render::renderer_2d_texture_id texture_index = 0ull;
        for (uint32_t i = 1; i < m_renderer_data.texture_slot_index; ++i)
        {
            // Dereference and compare the textures
            if (*m_renderer_data.texture_slots[i] == *p_texture)
            {
                texture_index = i;
                break;
            }
        }

        if (texture_index == 0ull)
        {
            texture_index = m_renderer_data.texture_slot_index;
            m_renderer_data.texture_slots[m_renderer_data.texture_slot_index++] = p_texture;
            KB_CORE_ASSERT(m_renderer_data.texture_slot_index < m_renderer_data.max_texture_slots, "texture slot overflow!")
        }

        return texture_index;
    }

    // submit batched quad data
    auto submit_quad_data(const Buffer& p_quad_buffer) noexcept -> void;

	// reset renderer2d stats
	void reset_stats();
	// get renderer2d stats
	renderer_2d_stats_t get_stats();
private:
	void start_new_batch() noexcept;
	void end_batch() noexcept;

    auto get_writeable_quad_buffer(u32 p_new_quad_count = 0) noexcept -> QuadVertex*&;
    auto get_writeable_circle_buffer(u32 p_new_circle_count = 0) noexcept -> CircleVertex*&;
    auto get_writeable_line_buffer(u32 p_new_line_count = 0) noexcept -> LineVertex*&;
    auto get_writeable_text_buffer(u32 p_new_text_count = 0) noexcept -> text_vertex_t*&;

    auto add_quad_buffer() noexcept -> void;
    auto add_circle_buffer() noexcept -> void;
    auto add_line_buffer() noexcept -> void;
    auto add_text_buffer() noexcept -> void;

private:
    renderer_2d_data_t m_renderer_data{};
    ref<asset::AssetManager> m_asset_manager{};
    bool m_explicit_render_pass_clear = false;
};

} // end namespace Kablunk

#endif
