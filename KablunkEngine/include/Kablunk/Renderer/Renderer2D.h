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

namespace Kablunk
{
	namespace render2d
	{
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
			std::vector<IntrusiveRef<VertexBuffer>> quad_vertex_buffers;
			IntrusiveRef <IndexBuffer> quad_index_buffer;

			// circle
			std::vector<IntrusiveRef<VertexBuffer>> circle_vertex_buffers;

			// lines
			std::vector<IntrusiveRef<VertexBuffer>> line_vertex_buffers;
			IntrusiveRef<IndexBuffer> line_index_buffer;

			// ui quads
			std::vector<IntrusiveRef<VertexBuffer>> ui_quad_vertex_buffers;
			IntrusiveRef<IndexBuffer> ui_quad_index_buffer;

			// text
			std::vector<IntrusiveRef<VertexBuffer>> ui_text_vertex_buffers;
			IntrusiveRef<IndexBuffer> ui_text_index_buffer;

			IntrusiveRef<Shader> quad_shader;
			IntrusiveRef<Shader> circle_shader;
			IntrusiveRef<Shader> line_shader;
			IntrusiveRef<Shader> ui_shader;

			IntrusiveRef <Texture2D> white_texture;

			// Quads
			std::vector<QuadVertex*> quad_vertex_buffer_base_ptrs;
			QuadVertex* quad_vertex_buffer_ptr = nullptr;
			uint32_t quad_count = 0;
			uint32_t quad_index_count = 0;

			// Circles
			std::vector<CircleVertex*> circle_vertex_buffer_base_ptrs;
			CircleVertex* circle_vertex_buffer_ptr = nullptr;
			uint32_t circle_count = 0;
			uint32_t circle_index_count = 0;

			// Lines
			std::vector<LineVertex*> line_vertex_buffer_base_ptrs;
			LineVertex* line_vertex_buffer_ptr = nullptr;
			uint32_t line_count = 0;
			uint32_t line_index_count = 0;
			float line_width = 1.0f;

			// UI
			std::vector<UIQuadVertex*> ui_quad_vertex_buffer_base_ptrs;
			UIQuadVertex* ui_quad_vertex_buffer_ptr = nullptr;
			uint32_t ui_quad_count = 0;
			uint32_t ui_quad_index_count = 0;

			uint32_t texture_slot_index = 1; //0 = white texture

			// TODO: change to asset handle when implemented
			std::array<IntrusiveRef<Texture2D>, max_texture_slots> texture_slots;

			IntrusiveRef<RenderCommandBuffer> render_command_buffer;

			IntrusiveRef<Pipeline> quad_pipeline;
			IntrusiveRef<Pipeline> circle_pipeline;
			IntrusiveRef<Pipeline> line_pipeline;
			IntrusiveRef<Pipeline> ui_pipeline;


			IntrusiveRef<Material> quad_material;
			IntrusiveRef<Material> circle_material;
			IntrusiveRef<Material> line_material;
			IntrusiveRef<Material> ui_material;

			IntrusiveRef<UniformBufferSet> uniform_buffer_set;

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
	} // end namespace renderer

	class Renderer2D : public RefCounted
	{
	public:
		Renderer2D() = default;
		~Renderer2D() = default;
		
		void init(render2d::renderer_2d_specification_t spec = {});
		void shutdown();

		IntrusiveRef<Texture2D> get_white_texture();

		void begin_scene(const Camera& camera, const glm::mat4& transform);
		void begin_scene(const EditorCamera& camera);
		void end_scene();
		void flush();

		void on_imgui_render();

		IntrusiveRef<RenderPass> get_target_render_pass();
		void set_target_render_pass(IntrusiveRef<RenderPass> render_pass);
		void on_recreate_swapchain();

		// Entity
		void draw_sprite(Entity entity);

		// Texture
		void draw_quad(const glm::vec2& position, const glm::vec2& size, const IntrusiveRef<Texture2D>& texture, float tilingFactor = 1.0f, const glm::vec4& tintColor = glm::vec4{ 1.0f });
		void draw_quad(const glm::vec3& position, const glm::vec2& size, const IntrusiveRef<Texture2D>& texture, float tilingFactor = 1.0f, const glm::vec4& tintColor = glm::vec4{ 1.0f });
		// #TODO figure out how to pass 64 bit integers to OpenGL so we can support int64_t instead of int32_t
		void draw_quad(const glm::mat4& transform, const IntrusiveRef<Texture2D>& texture, float tilingFactor = 1.0f, const glm::vec4& tintColor = glm::vec4{ 1.0f }, int32_t entity_id = -1);

		// Texture Atlas
		void draw_quad_from_texture_atlas(const glm::vec2& position, const glm::vec2& size, const IntrusiveRef<Texture2D>& texture, const glm::vec2* texture_atlas_offsets, float tiling_factor = 1.0f, const glm::vec4& tint_color = glm::vec4{ 1.0f });
		void draw_quad_from_texture_atlas(const glm::vec3& position, const glm::vec2& size, const IntrusiveRef<Texture2D>& texture, const glm::vec2* texture_atlas_offsets, float tiling_factor = 1.0f, const glm::vec4& tint_color = glm::vec4{ 1.0f });
		void draw_quad_from_texture_atlas(const glm::mat4& transform, const glm::vec2& size, const IntrusiveRef<Texture2D>& texture, const glm::vec2* texture_atlas_offsets, float tiling_factor = 1.0f, const glm::vec4& tint_color = glm::vec4{ 1.0f });

		void draw_circle(const glm::mat4& transform, const glm::vec4& color, float radius = 0.5f, float thickness = 1.0f, float fade = 0.005f, int32_t entity_id = -1);

		// Line
		void draw_line(const glm::vec3& p0, const glm::vec3& p1, const glm::vec4& color = glm::vec4{ 1.0f });

		// Rect
		void draw_rect(const glm::vec2& position, const glm::vec2& size, float rotation = 0, const glm::vec4& color = glm::vec4{ 1.0f });
		void draw_rect(const glm::vec3& position, const glm::vec2& size, float rotation = 0, const glm::vec4& color = glm::vec4{ 1.0f });

		// ui
		void draw_quad_ui(const glm::vec2& position, const glm::vec2& size, const IntrusiveRef<Texture2D>& texture, float tiling_factor = 1.0f, const glm::vec4& tint_color = glm::vec4{ 1.0f });
		void draw_quad_ui(const glm::vec3& position, const glm::vec2& size, const IntrusiveRef<Texture2D>& texture, float tiling_factor = 1.0f, const glm::vec4& tint_color = glm::vec4{ 1.0f });
		void draw_quad_ui(const glm::mat4& transform, const IntrusiveRef<Texture2D>& texture, float tiling_factor = 1.0f, const glm::vec4& tint_color = glm::vec4{ 1.0f });

		void draw_text_string(const std::string& text, const glm::vec2& position, const glm::vec2& size, const glm::vec4& tint_color = glm::vec4{ 1.0f });
		void draw_text_string(const std::string& text, const glm::vec3& position, const glm::vec2& size, const glm::vec4& tint_color = glm::vec4{ 1.0f });
		void draw_text_string(const std::string& text, const glm::mat4& position, const glm::vec2& size, const glm::vec4& tint_color = glm::vec4{ 1.0f });

		void reset_stats();
		render2d::renderer_2d_stats_t get_stats();
	private:
		void start_new_batch();
		void end_batch();
	private:
		render2d::renderer_2d_data_t* m_renderer_data = nullptr;
	};
}

#endif
