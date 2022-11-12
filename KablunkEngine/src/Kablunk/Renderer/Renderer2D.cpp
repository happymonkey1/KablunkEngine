#include "kablunkpch.h"
#include "Kablunk/Renderer/Renderer2D.h"

#include "Kablunk/Renderer/VertexArray.h"
#include "Kablunk/Renderer/Shader.h"
#include "Kablunk/Renderer/RenderCommand.h"
#include "Kablunk/Renderer/UniformBuffer.h"
#include "Kablunk/Renderer/Renderer.h"

#include "Kablunk/Scene/Components.h"
#include "Kablunk/Renderer/RendererAPI.h"

#include <glm/gtc/matrix_transform.hpp>
#include <imgui.h>


namespace Kablunk
{
	void Renderer2D::init(render2d::renderer_2d_specification_t spec)
	{
		KB_PROFILE_FUNCTION();

		m_renderer_data = new render2d::renderer_2d_data_t{};
		m_renderer_data->specification = spec;

		if (m_renderer_data->specification.swap_chain_target)
			m_renderer_data->render_command_buffer = RenderCommandBuffer::CreateFromSwapChain("Renderer2D");
		else
			m_renderer_data->render_command_buffer = RenderCommandBuffer::Create(0, "Renderer2D");

		uint32_t frames_in_flight = render::get_frames_in_flights();
		
		// =====
		// Quads
		// =====

		m_renderer_data->quad_vertex_buffers.resize(frames_in_flight);
		m_renderer_data->quad_vertex_buffer_base_ptrs.resize(frames_in_flight);
		for (size_t i = 0; i < frames_in_flight; ++i)
		{
			m_renderer_data->quad_vertex_buffers[i] = VertexBuffer::Create(m_renderer_data->max_vertices * sizeof(render2d::QuadVertex));
			m_renderer_data->quad_vertex_buffer_base_ptrs[i] = new render2d::QuadVertex[m_renderer_data->max_vertices];
		}

		uint32_t* quad_indices = new uint32_t[m_renderer_data->max_indices];

		uint32_t offset = 0;
		for (uint32_t i = 0; i < m_renderer_data->max_indices; i += 6)
		{
			quad_indices[i + uint32_t{ 0 }] = offset + uint32_t{ 0 };
			quad_indices[i + uint32_t{ 1 }] = offset + uint32_t{ 1 };
			quad_indices[i + uint32_t{ 2 }] = offset + uint32_t{ 2 };

			quad_indices[i + uint32_t{ 3 }] = offset + uint32_t{ 2 };
			quad_indices[i + uint32_t{ 4 }] = offset + uint32_t{ 3 };
			quad_indices[i + uint32_t{ 5 }] = offset + uint32_t{ 0 };

			offset += 4;
		}

		m_renderer_data->quad_index_buffer = IndexBuffer::Create(quad_indices, m_renderer_data->max_indices);
		delete[] quad_indices;

		// ========
		// UI Quads
		// ========

		m_renderer_data->ui_quad_vertex_buffers.resize(frames_in_flight);
		m_renderer_data->ui_quad_vertex_buffer_base_ptrs.resize(frames_in_flight);
		for (size_t i = 0; i < frames_in_flight; ++i)
		{
			m_renderer_data->ui_quad_vertex_buffers[i] = VertexBuffer::Create(m_renderer_data->max_vertices * sizeof(render2d::UIQuadVertex));
			m_renderer_data->ui_quad_vertex_buffer_base_ptrs[i] = new render2d::UIQuadVertex[m_renderer_data->max_vertices];
		}

		// =======
		// Circles
		// =======

		m_renderer_data->circle_vertex_buffers.resize(frames_in_flight);
		m_renderer_data->circle_vertex_buffer_base_ptrs.resize(frames_in_flight);
		for (size_t i = 0; i < frames_in_flight; ++i)
		{
			m_renderer_data->circle_vertex_buffers[i] = VertexBuffer::Create(m_renderer_data->max_vertices * sizeof(render2d::CircleVertex));
			m_renderer_data->circle_vertex_buffer_base_ptrs[i] = new render2d::CircleVertex[m_renderer_data->max_vertices];
		}

		// =====
		// Lines
		// =====

		m_renderer_data->line_vertex_buffers.resize(frames_in_flight);
		m_renderer_data->line_vertex_buffer_base_ptrs.resize(frames_in_flight);
		for (size_t i = 0; i < frames_in_flight; ++i)
		{
			m_renderer_data->line_vertex_buffers[i] = VertexBuffer::Create(m_renderer_data->max_vertices * sizeof(render2d::LineVertex));
			m_renderer_data->line_vertex_buffer_base_ptrs[i] = new render2d::LineVertex[m_renderer_data->max_vertices];
		}

		uint32_t white_texture_data = 0xFFFFFFFF;
		m_renderer_data->white_texture = Texture2D::Create(ImageFormat::RGBA, 1, 1, &white_texture_data);

		m_renderer_data->quad_shader = render::get_shader("Renderer2D_Quad");
		m_renderer_data->circle_shader = render::get_shader("Renderer2D_Circle");
		m_renderer_data->line_shader = render::get_shader("Renderer2D_Line");
		m_renderer_data->ui_shader = render::get_shader("Renderer2D_UI");

		// Set all the texture slots to zero
		//memset(s_RendererData.TextureSlots.data(), 0, s_RendererData.TextureSlots.size() * sizeof(uint32_t));

		m_renderer_data->texture_slots[0] = m_renderer_data->white_texture;

		m_renderer_data->quad_vertex_positions[0] = { -0.5f, -0.5f, 0.0f, 1.0f };
		m_renderer_data->quad_vertex_positions[1] = { 0.5f, -0.5f, 0.0f, 1.0f };
		m_renderer_data->quad_vertex_positions[2] = { 0.5f,  0.5f, 0.0f, 1.0f };
		m_renderer_data->quad_vertex_positions[3] = { -0.5f,  0.5f, 0.0f, 1.0f };

		// Create framebuffer
		FramebufferSpecification framebuffer_spec{};
		framebuffer_spec.Attachments = { ImageFormat::RGBA, ImageFormat::Depth };
		framebuffer_spec.samples = 1;
		framebuffer_spec.clear_on_load = false;
		framebuffer_spec.clear_color = { 0.5f, 0.1f, 0.1f, 1.0f };
		framebuffer_spec.debug_name = "Renderer2D Framebuffer";
		framebuffer_spec.blend_mode = FramebufferBlendMode::Additive;
		framebuffer_spec.blend = true;

		IntrusiveRef<Framebuffer> framebuffer = Framebuffer::Create(framebuffer_spec);

		RenderPassSpecification render_pass_spec{};
		render_pass_spec.target_framebuffer = framebuffer;
		render_pass_spec.debug_name = "Renderer2D";

		IntrusiveRef<RenderPass> render_pass = RenderPass::Create(render_pass_spec);

		// Create quad pipeline
		{
			PipelineSpecification pipeline_spec;
			pipeline_spec.debug_name = "QuadPipeline";
			pipeline_spec.shader = render::get_shader("Renderer2D_Quad");
			pipeline_spec.backface_culling = false;
			pipeline_spec.layout = {
				{ ShaderDataType::Float3, "a_Position" },
				{ ShaderDataType::Float4, "a_Color" },
				{ ShaderDataType::Float2, "a_TexCoord" },
				{ ShaderDataType::Float, "a_TexIndex" },
				{ ShaderDataType::Float, "a_TilingFactor" },
				{ ShaderDataType::Int, "a_EntityID" }
			};
			pipeline_spec.render_pass = render_pass;

			m_renderer_data->quad_pipeline = Pipeline::Create(pipeline_spec);
		}

		// UI
		{
			PipelineSpecification pipeline_spec;
			pipeline_spec.debug_name = "UIPipeline";
			pipeline_spec.shader = render::get_shader("Renderer2D_UI");
			pipeline_spec.backface_culling = false;
			pipeline_spec.layout = {
				{ ShaderDataType::Float3, "a_Position" },
				{ ShaderDataType::Float4, "a_Color" },
				{ ShaderDataType::Float2, "a_TextCoord" },
				{ ShaderDataType::Float, "a_TexIndex" },
				{ ShaderDataType::Float, "a_TilingFactor" }
			};
			pipeline_spec.render_pass = render_pass;

			m_renderer_data->ui_pipeline = Pipeline::Create(pipeline_spec);
		}

		// Circle
		{
			PipelineSpecification pipeline_spec;
			pipeline_spec.debug_name = "CirclePipeline";
			pipeline_spec.shader = render::get_shader("Renderer2D_Circle");
			pipeline_spec.backface_culling = false;
			pipeline_spec.layout = {
				{ ShaderDataType::Float3, "a_WorldPosition" },
				{ ShaderDataType::Float3, "a_LocalPosition" },
				{ ShaderDataType::Float4, "a_Color"},
				{ ShaderDataType::Float, "a_Radius" },
				{ ShaderDataType::Float, "a_Thickness" },
				{ ShaderDataType::Float, "a_Fade" },
				{ ShaderDataType::Int, "a_EntityID" }
			};
			pipeline_spec.render_pass = render_pass;

			m_renderer_data->circle_pipeline = Pipeline::Create(pipeline_spec);
		}

		// Line
		{
			PipelineSpecification pipeline_spec;
			pipeline_spec.debug_name = "LinePipeline";
			pipeline_spec.shader = render::get_shader("Renderer2D_Line");
			pipeline_spec.backface_culling = false;
			pipeline_spec.layout = {
				{ ShaderDataType::Float3, "a_Position" },
				{ ShaderDataType::Float4, "a_Color" }
			};
			pipeline_spec.render_pass = render_pass;

			m_renderer_data->line_pipeline = Pipeline::Create(pipeline_spec);

			uint32_t* line_indices = new uint32_t[m_renderer_data->max_line_indices];
			for (uint32_t i = 0; i < m_renderer_data->max_line_indices; ++i)
				line_indices[i] = i;

			m_renderer_data->line_index_buffer = IndexBuffer::Create(line_indices, m_renderer_data->max_line_indices);
			delete[] line_indices;
		}

		m_renderer_data->quad_material = Material::Create(m_renderer_data->quad_shader);
		m_renderer_data->circle_material = Material::Create(m_renderer_data->circle_shader);
		m_renderer_data->line_material = Material::Create(m_renderer_data->line_shader);
		m_renderer_data->ui_material = Material::Create(m_renderer_data->ui_shader);

		m_renderer_data->uniform_buffer_set = UniformBufferSet::Create(frames_in_flight);
		m_renderer_data->uniform_buffer_set->Create(sizeof(CameraDataUB), 0);
	}

	void Renderer2D::shutdown()
	{
		KB_PROFILE_FUNCTION();

		KB_CORE_INFO("Shutting down Renderer2D!");

		// Cleanup memory
		for (auto buffer : m_renderer_data->quad_vertex_buffer_base_ptrs)
			delete[] buffer;

		for (auto buffer : m_renderer_data->circle_vertex_buffer_base_ptrs)
			delete[] buffer;

		for (auto buffer : m_renderer_data->line_vertex_buffer_base_ptrs)
			delete[] buffer;

		delete m_renderer_data;
	}

	IntrusiveRef<Texture2D> Renderer2D::get_white_texture()
	{
		return m_renderer_data->white_texture;
	}

	void Renderer2D::begin_scene(const Camera& camera, const glm::mat4& transform)
	{
		m_renderer_data->camera = camera;
		m_renderer_data->camera_transform = transform;

		glm::mat4 view_proj = camera.GetProjection() * transform;
		m_renderer_data->camera_view_projection = view_proj;

		CameraDataUB camera_data_ub = CameraDataUB{
			view_proj,
			camera.GetProjection(),
			transform,
			glm::vec3{ 1.0f } // #TODO fix
		};

		IntrusiveRef<UniformBufferSet> uniform_buffer_set = m_renderer_data->uniform_buffer_set;
		render::submit([uniform_buffer_set, camera_data_ub]() mutable
			{
				uint32_t buffer_index = render::get_current_frame_index();
				uniform_buffer_set->Get(0, 0, buffer_index)->RT_SetData(&camera_data_ub, sizeof(CameraDataUB));
			});

		m_renderer_data->Stats = {};

		start_new_batch();
	}

	void Renderer2D::begin_scene(const EditorCamera& camera)
	{
		Renderer2D::begin_scene(camera, camera.GetViewMatrix());

		start_new_batch();
	}

	void Renderer2D::end_scene()
	{
		flush();
	}

	void Renderer2D::flush()
	{
		KB_CORE_ASSERT(m_renderer_data->Stats.batch_count < 1, "Multiple batches per frame not supported!");

		m_renderer_data->render_command_buffer->Begin();

		m_renderer_data->gpu_time_query.renderer_2D_query = m_renderer_data->render_command_buffer->BeginTimestampQuery();
		render::begin_render_pass(m_renderer_data->render_command_buffer, m_renderer_data->quad_pipeline->GetSpecification().render_pass);

		uint32_t frame_index = render::get_current_frame_index();

		// Quad
		// calculate data size in bytes
		uint32_t data_size = (uint32_t)((uint8_t*)m_renderer_data->quad_vertex_buffer_ptr - (uint8_t*)m_renderer_data->quad_vertex_buffer_base_ptrs[frame_index]);
		if (data_size)
		{
			m_renderer_data->quad_vertex_buffers[frame_index]->SetData(m_renderer_data->quad_vertex_buffer_base_ptrs[frame_index], data_size);

			// Set Textures
			auto& textures = m_renderer_data->texture_slots;
			for (uint32_t i = 0; i < m_renderer_data->max_texture_slots; i++)
			{
				if (textures[i])
					m_renderer_data->quad_material->Set("u_Textures", textures[i], i);
				else
					m_renderer_data->quad_material->Set("u_Textures", m_renderer_data->white_texture, i);
			}

			render::render_geometry(m_renderer_data->render_command_buffer, m_renderer_data->quad_pipeline, m_renderer_data->uniform_buffer_set, nullptr, m_renderer_data->quad_material, m_renderer_data->quad_vertex_buffers[frame_index], m_renderer_data->quad_index_buffer, glm::mat4{ 1.0f }, m_renderer_data->quad_index_count);
			m_renderer_data->Stats.Draw_calls++;
		}

		// Circle
		data_size = (uint32_t)((uint8_t*)m_renderer_data->circle_vertex_buffer_ptr - (uint8_t*)m_renderer_data->circle_vertex_buffer_base_ptrs[frame_index]);
		if (data_size)
		{
			m_renderer_data->circle_vertex_buffers[frame_index]->SetData(m_renderer_data->circle_vertex_buffer_base_ptrs[frame_index], data_size);

			render::render_geometry(m_renderer_data->render_command_buffer, m_renderer_data->circle_pipeline, m_renderer_data->uniform_buffer_set, nullptr, m_renderer_data->circle_material, m_renderer_data->circle_vertex_buffers[frame_index], m_renderer_data->quad_index_buffer, glm::mat4{1.0f}, m_renderer_data->circle_index_count);
			m_renderer_data->Stats.Draw_calls++;
		}

		// Line
		data_size = (uint32_t)((uint8_t*)m_renderer_data->line_vertex_buffer_ptr - (uint8_t*)m_renderer_data->line_vertex_buffer_base_ptrs[frame_index]);
		if (data_size)
		{
			m_renderer_data->line_vertex_buffers[frame_index]->SetData(m_renderer_data->line_vertex_buffer_base_ptrs[frame_index], data_size);

			render::set_line_width(m_renderer_data->render_command_buffer, m_renderer_data->line_width);

			render::render_geometry(m_renderer_data->render_command_buffer, m_renderer_data->line_pipeline, m_renderer_data->uniform_buffer_set, nullptr, m_renderer_data->line_material, m_renderer_data->line_vertex_buffers[frame_index], m_renderer_data->line_index_buffer, glm::mat4{1.0f}, m_renderer_data->line_index_count);
			m_renderer_data->Stats.Draw_calls++;
		}

		// UI
		// calculate data size in bytes
		data_size = (uint32_t)((uint8_t*)m_renderer_data->ui_quad_vertex_buffer_ptr - (uint8_t*)m_renderer_data->ui_quad_vertex_buffer_base_ptrs[frame_index]);
		if (data_size)
		{
			m_renderer_data->ui_quad_vertex_buffers[frame_index]->SetData(m_renderer_data->ui_quad_vertex_buffer_base_ptrs[frame_index], data_size);

			// Set Textures
			auto& textures = m_renderer_data->texture_slots;
			for (uint32_t i = 0; i < m_renderer_data->max_texture_slots; i++)
			{
				if (textures[i])
					m_renderer_data->ui_material->Set("u_Textures", textures[i], i);
				else
					m_renderer_data->ui_material->Set("u_Textures", m_renderer_data->white_texture, i);
			}

			render::render_geometry(m_renderer_data->render_command_buffer, m_renderer_data->ui_pipeline, m_renderer_data->uniform_buffer_set, nullptr, m_renderer_data->ui_material, m_renderer_data->ui_quad_vertex_buffers[frame_index], m_renderer_data->quad_index_buffer, glm::mat4{ 1.0f }, m_renderer_data->ui_quad_index_count);
			m_renderer_data->Stats.Draw_calls++;
		}

		render::end_render_pass(m_renderer_data->render_command_buffer);
		m_renderer_data->render_command_buffer->EndTimestampQuery(m_renderer_data->gpu_time_query.renderer_2D_query);

		m_renderer_data->render_command_buffer->End();
		m_renderer_data->render_command_buffer->Submit();

		m_renderer_data->Stats.batch_count++;
	}

	void Renderer2D::on_imgui_render()
	{
		uint32_t current_frame_index = render::get_current_frame_index();
		ImGui::Text("2D Geometry Pass: %.3fms", m_renderer_data->render_command_buffer->GetExecutionGPUTime(current_frame_index, static_cast<uint32_t>(m_renderer_data->gpu_time_query.renderer_2D_query)));
	}

	IntrusiveRef<RenderPass> Renderer2D::get_target_render_pass()
	{
		return m_renderer_data->quad_pipeline->GetSpecification().render_pass;
	}

	void Renderer2D::set_target_render_pass(IntrusiveRef<RenderPass> render_pass)
	{
		if (m_renderer_data->quad_pipeline->GetSpecification().render_pass != render_pass)
		{
			// Quad pipeline
			{
				PipelineSpecification pipeline_spec = m_renderer_data->quad_pipeline->GetSpecification();
				pipeline_spec.render_pass = render_pass;
				m_renderer_data->quad_pipeline = Pipeline::Create(pipeline_spec);
			}

			// Circle pipeline
			{
				PipelineSpecification pipeline_spec = m_renderer_data->circle_pipeline->GetSpecification();
				pipeline_spec.render_pass = render_pass;
				m_renderer_data->circle_pipeline = Pipeline::Create(pipeline_spec);
			}
		}
	}

	void Renderer2D::on_recreate_swapchain()
	{
		if (m_renderer_data->specification.swap_chain_target)
			m_renderer_data->render_command_buffer = RenderCommandBuffer::CreateFromSwapChain("Renderer2D");
	}

	// =========================
	//   Draw Quad From Entity
	// =========================

	void Renderer2D::draw_sprite(Entity entity)
	{
		auto transform = entity.m_scene->get_world_space_transform_matrix(entity);

		auto& sprite_renderer_comp = entity.GetComponent<SpriteRendererComponent>();

		const auto& texture = sprite_renderer_comp.Texture.Get();
		auto tint_color = sprite_renderer_comp.Color;
		auto tiling_factor = sprite_renderer_comp.Tiling_factor;

		draw_quad(transform, texture, tiling_factor, tint_color, static_cast<int32_t>(entity.GetHandle()));
	}

	// ==========================
	//   Draw Quad with Texture
	// ==========================

	void Renderer2D::draw_quad(const glm::vec2& position, const glm::vec2& size, const IntrusiveRef<Texture2D>& texture, float tiling_factor, const glm::vec4& tint_color)
	{
		draw_quad({ position.x, position.y, 0.0f }, size, texture, tiling_factor, tint_color);
	}

	void Renderer2D::draw_quad(const glm::vec3& position, const glm::vec2& size, const IntrusiveRef<Texture2D>& texture, float tiling_factor, const glm::vec4& tint_color)
	{
		glm::mat4 transform = glm::translate(glm::mat4(1.0f), position)
			* glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });

		draw_quad(transform, texture, tiling_factor, tint_color);
	}

	void Renderer2D::draw_quad(const glm::mat4& transform, const IntrusiveRef<Texture2D>& texture, float tiling_factor, const glm::vec4& tint_color, int32_t entity_id)
	{
		if (m_renderer_data->quad_count + 1 > m_renderer_data->max_quads)
			end_batch();

		//constexpr glm::vec4 color{ 1.0f, 1.0f, 1.0f, 1.0f };
		float texture_index = 0.0f;
		for (uint32_t i = 1; i < m_renderer_data->texture_slot_index; ++i)
		{
			// Dereference shared_ptrs and compare the textures
			if (*m_renderer_data->texture_slots[i].get() == *texture.get())
				texture_index = (float)i;
		}

		if (texture_index == 0.0f)
		{
			texture_index = (float)m_renderer_data->texture_slot_index;
			m_renderer_data->texture_slots[m_renderer_data->texture_slot_index++] = texture;
			KB_CORE_ASSERT(m_renderer_data->texture_slot_index < m_renderer_data->max_texture_slots, "texture slot overflow!");
		}

		constexpr glm::vec2 texture_coords[] = { {0.0f, 0.0f}, { 1.0f, 0.0f}, { 1.0f, 1.0f}, { 0.0f, 1.0f } };
		constexpr size_t quad_vertex_count = 4;

		for (uint32_t i = 0; i < quad_vertex_count; ++i)
		{
			m_renderer_data->quad_vertex_buffer_ptr->Position = transform * m_renderer_data->quad_vertex_positions[i];
			m_renderer_data->quad_vertex_buffer_ptr->Color = tint_color;
			m_renderer_data->quad_vertex_buffer_ptr->TexCoord = texture_coords[i];
			m_renderer_data->quad_vertex_buffer_ptr->TexIndex = texture_index;
			m_renderer_data->quad_vertex_buffer_ptr->TilingFactor = tiling_factor;
			m_renderer_data->quad_vertex_buffer_ptr->EntityID = entity_id;
			m_renderer_data->quad_vertex_buffer_ptr++;
		}
		m_renderer_data->quad_index_count += 6;
		m_renderer_data->quad_count++;

		m_renderer_data->Stats.Quad_count += 1;
	}

	// DrawQuadTextureAtlas
	void Renderer2D::draw_quad_from_texture_atlas(const glm::vec2& position, const glm::vec2& size, const IntrusiveRef<Texture2D>& texture, const glm::vec2* texture_atlas_offsets, float tiling_factor, const glm::vec4& tint_color)
	{
		draw_quad_from_texture_atlas(glm::vec3{ position.x, position.y, 0.0f }, size, texture, texture_atlas_offsets, tiling_factor, tint_color);
	}
	void Renderer2D::draw_quad_from_texture_atlas(const glm::vec3& position, const glm::vec2& size, const IntrusiveRef<Texture2D>& texture, const glm::vec2* texture_atlas_offsets, float tiling_factor, const glm::vec4& tint_color)
	{
		glm::mat4 transform = glm::translate(glm::mat4{ 1.0f }, position)
			* glm::rotate(glm::mat4{ 1.0f }, 0.0f, { 0.0f, 0.0f, 1.0f })
			* glm::scale(glm::mat4{ 1.0f }, { size.x, size.y, 1.0f });

		draw_quad_from_texture_atlas(transform, size, texture, texture_atlas_offsets, tiling_factor, tint_color);
	}
	void Renderer2D::draw_quad_from_texture_atlas(const glm::mat4& transform, const glm::vec2& size, const IntrusiveRef<Texture2D>& texture, const glm::vec2* texture_atlas_offsets, float tiling_factor, const glm::vec4& tint_color)
	{
		if (m_renderer_data->quad_count + 1 > m_renderer_data->max_quads)
			end_batch();

		//constexpr glm::vec4 color{ 1.0f, 1.0f, 1.0f, 1.0f };
		float texture_index = 0.0f;
		for (uint32_t i = 1; i < m_renderer_data->texture_slot_index; ++i)
		{
			// Dereference and compare the textures
			if (*m_renderer_data->texture_slots[i].get() == *texture.get())
				texture_index = (float)i;
		}

		if (texture_index == 0.0f)
		{
			texture_index = (float)m_renderer_data->texture_slot_index;
			m_renderer_data->texture_slots[m_renderer_data->texture_slot_index++] = texture;
			KB_CORE_ASSERT(m_renderer_data->texture_slot_index < m_renderer_data->max_texture_slots, "texture slot overflow!");
		}

		constexpr size_t quad_vertex_count = 4;

		for (uint32_t i = 0; i < quad_vertex_count; ++i)
		{
			m_renderer_data->quad_vertex_buffer_ptr->Position = transform * m_renderer_data->quad_vertex_positions[i];
			m_renderer_data->quad_vertex_buffer_ptr->Color = tint_color;
			m_renderer_data->quad_vertex_buffer_ptr->TexCoord = texture_atlas_offsets[i];
			m_renderer_data->quad_vertex_buffer_ptr->TexIndex = texture_index;
			m_renderer_data->quad_vertex_buffer_ptr->TilingFactor = tiling_factor;
			m_renderer_data->quad_vertex_buffer_ptr->EntityID = 0;
			m_renderer_data->quad_vertex_buffer_ptr++;
		}
		m_renderer_data->quad_index_count += 6;
		m_renderer_data->quad_count++;

		m_renderer_data->Stats.Quad_count += 1;
	}

	void Renderer2D::draw_circle(const glm::mat4& transform, const glm::vec4& color, float radius /*= 0.5f*/, float thickness /*= 1.0f*/, float fade /*= 0.005f*/, int32_t entity_id /*= -1*/)
	{
		// #TODO implement for circles
		// if (m_renderer_data->Quad_count + 1 > m_renderer_data->Max_quads)
		// 	EndBatch();

		constexpr size_t circle_vertex_count = 4;

		for (uint32_t i = 0; i < circle_vertex_count; ++i)
		{
			m_renderer_data->circle_vertex_buffer_ptr->WorldPosition = transform * m_renderer_data->quad_vertex_positions[i];
			m_renderer_data->circle_vertex_buffer_ptr->LocalPosition = m_renderer_data->quad_vertex_positions[i] * 2.0f;
			m_renderer_data->circle_vertex_buffer_ptr->Color = color;
			m_renderer_data->circle_vertex_buffer_ptr->Radius = radius;
			m_renderer_data->circle_vertex_buffer_ptr->Thickness = thickness;
			m_renderer_data->circle_vertex_buffer_ptr->Fade = fade;
			m_renderer_data->circle_vertex_buffer_ptr->EntityID = entity_id;
			m_renderer_data->circle_vertex_buffer_ptr++;
		}
		m_renderer_data->circle_index_count += 6;
		m_renderer_data->circle_count++;

		m_renderer_data->Stats.Circle_count += 1;
	}

	void Renderer2D::draw_line(const glm::vec3& p0, const glm::vec3& p1, const glm::vec4& color /*= glm::vec4{ 1.0f }*/)
	{
		if (m_renderer_data->line_index_count >= m_renderer_data->max_line_indices)
			end_batch();

		m_renderer_data->line_vertex_buffer_ptr->Position = p0;
		m_renderer_data->line_vertex_buffer_ptr->Color = color;
		m_renderer_data->line_vertex_buffer_ptr++;

		m_renderer_data->line_vertex_buffer_ptr->Position = p1;
		m_renderer_data->line_vertex_buffer_ptr->Color = color;
		m_renderer_data->line_vertex_buffer_ptr++;

		m_renderer_data->line_index_count += 2;
		m_renderer_data->line_count += 1;
		
		// #TODO statistics
	}

	void Renderer2D::draw_rect(const glm::vec2& position, const glm::vec2& size, float rotation /* = 0 */, const glm::vec4& color /* = glm::vec4 */)
	{
		Renderer2D::draw_rect(glm::vec3{ position.x, position.y, 0.0f }, size, rotation, color);
	}

	void Renderer2D::draw_rect(const glm::vec3& position, const glm::vec2& size, float rotation /* = 0 */, const glm::vec4& color /* = glm::vec4 */)
	{
		if (m_renderer_data->line_index_count >= m_renderer_data->max_line_indices)
			end_batch();

		glm::mat4 transform = glm::translate(glm::mat4{ 1.0f }, position)
			* glm::rotate(glm::mat4{ 1.0f }, rotation, { 0.0f, 0.0f, 1.0f })
			* glm::scale(glm::mat4{ 1.0f }, glm::vec3{ size.x, size.y, 1.0f });

		glm::vec3 positions[4] =
		{
			transform * m_renderer_data->quad_vertex_positions[0],
			transform * m_renderer_data->quad_vertex_positions[1],
			transform * m_renderer_data->quad_vertex_positions[2],
			transform * m_renderer_data->quad_vertex_positions[3]
		};

		for (uint32_t i = 0; i < 4; ++i)
		{
			auto& p0 = positions[i];
			auto& p1 = positions[(i + 1) % 4];

			m_renderer_data->line_vertex_buffer_ptr->Position = p0;
			m_renderer_data->line_vertex_buffer_ptr->Color = color;
			m_renderer_data->line_vertex_buffer_ptr++;

			m_renderer_data->line_vertex_buffer_ptr->Position = p1;
			m_renderer_data->line_vertex_buffer_ptr->Color = color;
			m_renderer_data->line_vertex_buffer_ptr++;

			m_renderer_data->line_index_count += 2;
			m_renderer_data->line_count += 1;
		}
	}

	void Renderer2D::draw_quad_ui(const glm::vec2& position, const glm::vec2& size, const IntrusiveRef<Texture2D>& texture, float tiling_factor, const glm::vec4& tint_color)
	{
		draw_quad_ui({ position.x, position.y, 0.0f }, size, texture, tiling_factor, tint_color);
	}

	void Renderer2D::draw_quad_ui(const glm::vec3& position, const glm::vec2& size, const IntrusiveRef<Texture2D>& texture, float tiling_factor, const glm::vec4& tint_color)
	{
		glm::mat4 transform = glm::translate(glm::mat4(1.0f), position)
			* glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });

		draw_quad_ui(transform, texture, tiling_factor, tint_color);
	}

	void Renderer2D::draw_quad_ui(const glm::mat4& transform, const IntrusiveRef<Texture2D>& texture, float tiling_factor, const glm::vec4& tint_color)
	{
		if (m_renderer_data->ui_quad_count + 1 > m_renderer_data->max_quads)
			end_batch();

		//constexpr glm::vec4 color{ 1.0f, 1.0f, 1.0f, 1.0f };
		float texture_index = 0.0f;
		for (uint32_t i = 1; i < m_renderer_data->texture_slot_index; ++i)
		{
			// Dereference shared_ptrs and compare the textures
			if (*m_renderer_data->texture_slots[i].get() == *texture.get())
				texture_index = (float)i;
		}

		if (texture_index == 0.0f)
		{
			texture_index = (float)m_renderer_data->texture_slot_index;
			m_renderer_data->texture_slots[m_renderer_data->texture_slot_index++] = texture;
			KB_CORE_ASSERT(m_renderer_data->texture_slot_index < m_renderer_data->max_texture_slots, "texture slot overflow!");
		}

		constexpr glm::vec2 texture_coords[] = { {0.0f, 0.0f}, { 1.0f, 0.0f}, { 1.0f, 1.0f}, { 0.0f, 1.0f } };
		constexpr size_t quad_vertex_count = 4;

		for (uint32_t i = 0; i < quad_vertex_count; ++i)
		{
			m_renderer_data->ui_quad_vertex_buffer_ptr->Position = transform * m_renderer_data->quad_vertex_positions[i];
			m_renderer_data->ui_quad_vertex_buffer_ptr->Color = tint_color;
			m_renderer_data->ui_quad_vertex_buffer_ptr->TexCoord = texture_coords[i];
			m_renderer_data->ui_quad_vertex_buffer_ptr->TexIndex = texture_index;
			m_renderer_data->ui_quad_vertex_buffer_ptr->TilingFactor = tiling_factor;
			m_renderer_data->ui_quad_vertex_buffer_ptr++;
		}
		m_renderer_data->ui_quad_index_count += 6;
		m_renderer_data->ui_quad_count++;

		m_renderer_data->Stats.Quad_count += 1;
	}

	void Renderer2D::reset_stats()
	{
		m_renderer_data->Stats.Draw_calls = 0;
		m_renderer_data->Stats.Quad_count = 0;
		m_renderer_data->Stats.Circle_count = 0;
	}

	render2d::renderer_2d_stats_t Renderer2D::get_stats() { return m_renderer_data->Stats; }

	void Renderer2D::start_new_batch()
	{
		uint32_t frame_index = render::get_current_frame_index();

		m_renderer_data->quad_count = 0;
		m_renderer_data->quad_index_count = 0;
		m_renderer_data->quad_vertex_buffer_ptr = m_renderer_data->quad_vertex_buffer_base_ptrs[frame_index];

		m_renderer_data->circle_count = 0;
		m_renderer_data->circle_index_count = 0;
		m_renderer_data->circle_vertex_buffer_ptr = m_renderer_data->circle_vertex_buffer_base_ptrs[frame_index];

		m_renderer_data->line_count = 0;
		m_renderer_data->line_index_count = 0;
		m_renderer_data->line_vertex_buffer_ptr = m_renderer_data->line_vertex_buffer_base_ptrs[frame_index];

		m_renderer_data->ui_quad_count = 0;
		m_renderer_data->ui_quad_index_count = 0;
		m_renderer_data->ui_quad_vertex_buffer_ptr = m_renderer_data->ui_quad_vertex_buffer_base_ptrs[frame_index];

		m_renderer_data->texture_slot_index = 1;

		for (size_t i = 0; i < m_renderer_data->max_texture_slots; ++i)
			if (i != 0)
				m_renderer_data->texture_slots[i] = nullptr;
	}

	void Renderer2D::end_batch()
	{
		flush();
		start_new_batch();
	}

}
