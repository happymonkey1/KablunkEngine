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
    KB_PROFILE_FUNC();

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

	// ====
	// text
	// ====

	m_renderer_data->text_vertex_buffers.resize(frames_in_flight);
	m_renderer_data->text_vertex_buffer_base_ptrs.resize(frames_in_flight);
	for (size_t i = 0; i < frames_in_flight; ++i)
	{
		// #TODO(Sean) this should probably be a separate max vertex count: `max_text_vertices`
		m_renderer_data->text_vertex_buffers[i] = VertexBuffer::Create(m_renderer_data->max_vertices * sizeof(render2d::text_vertex_t));
		m_renderer_data->text_vertex_buffer_base_ptrs[i] = new render2d::text_vertex_t[m_renderer_data->max_vertices];
	}

	uint32_t white_texture_data = 0xFFFFFFFF;
	m_renderer_data->white_texture = Texture2D::Create(ImageFormat::RGBA, 1, 1, &white_texture_data);

	// get references to pre-loaded shaders
	m_renderer_data->quad_shader = render::get_shader("Renderer2D_Quad");
	m_renderer_data->circle_shader = render::get_shader("Renderer2D_Circle");
	m_renderer_data->line_shader = render::get_shader("Renderer2D_Line");
	m_renderer_data->ui_shader = render::get_shader("Renderer2D_UI");
	m_renderer_data->text_shader = render::get_shader("Renderer2D_Text");

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
		pipeline_spec.shader = m_renderer_data->quad_shader;
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
		pipeline_spec.shader = m_renderer_data->ui_shader;
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
		pipeline_spec.shader = m_renderer_data->circle_shader;
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
		pipeline_spec.shader = m_renderer_data->line_shader;
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

	// create text pipeline
	{
		PipelineSpecification pipeline_spec;
		pipeline_spec.debug_name = "TextPipeline";
		pipeline_spec.shader = m_renderer_data->text_shader;
		pipeline_spec.backface_culling = false;
		pipeline_spec.layout = {
			{ ShaderDataType::Float3, "a_Position" },
			{ ShaderDataType::Float4, "a_Color" },
			{ ShaderDataType::Float2, "a_TexCoord" },
			{ ShaderDataType::Float, "a_TexIndex" },
		};
		pipeline_spec.render_pass = render_pass;

		m_renderer_data->text_pipeline = Pipeline::Create(pipeline_spec);
	}

	// create materials
	m_renderer_data->quad_material = Material::Create(m_renderer_data->quad_shader);
	m_renderer_data->circle_material = Material::Create(m_renderer_data->circle_shader);
	m_renderer_data->line_material = Material::Create(m_renderer_data->line_shader);
	m_renderer_data->ui_material = Material::Create(m_renderer_data->ui_shader);
	m_renderer_data->text_material = Material::Create(m_renderer_data->text_shader);

	m_renderer_data->uniform_buffer_set = UniformBufferSet::Create(frames_in_flight);
	m_renderer_data->uniform_buffer_set->Create(sizeof(CameraDataUB), 0);

	// initialize font manager
	m_renderer_data->m_font_manager.init();

	// =====================
	// pre-load engine fonts
	// =====================

    // #NOTE(Sean B) renderer is initialized during engine startup, while asset manager is (currently) initialized when a project is loaded,
    //               this means we CANNOT load an asset now. 
    //               initialization of default font asset(s) have moved to asset init function
#if 0
	// roboto-medium.ttf
	render::font_asset_create_info_t font_create_info{
		"resources/fonts/roboto/Roboto-Medium.ttf",			// relative path to font file
		16,													// font point
		m_renderer_data->m_font_manager.get_ft_engine(),	// freetype engine
		0ull,												// font face index
		128,												// number of glyphs to load
		true												// flag for whether font should be loaded into memory
	};
	ref<render::font_asset_t> font_asset = render::font_asset_t::create(font_create_info);

	// add fonts to font manager
	m_renderer_data->m_font_manager.add_font_file_to_library(font_asset);
#endif

	// =====================
}

void Renderer2D::shutdown()
{
    KB_PROFILE_FUNC();

	KB_CORE_INFO("Shutting down Renderer2D!");

	// free quad vertex buffers
	for (auto buffer : m_renderer_data->quad_vertex_buffer_base_ptrs)
		delete[] buffer;

	// free circle vertex buffers
	for (auto buffer : m_renderer_data->circle_vertex_buffer_base_ptrs)
		delete[] buffer;

	// free line vertex buffers
	for (auto buffer : m_renderer_data->line_vertex_buffer_base_ptrs)
		delete[] buffer;

	// free text vertex buffers
	for (auto buffer : m_renderer_data->text_vertex_buffer_base_ptrs)
		delete[] buffer;

	delete m_renderer_data;
}

IntrusiveRef<Texture2D> Renderer2D::get_white_texture()
{
	return m_renderer_data->white_texture;
}

void Renderer2D::begin_scene(const Camera& camera, const glm::mat4& transform)
{
    KB_PROFILE_FUNC();

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
			uint32_t buffer_index = render::rt_get_current_frame_index();
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
    KB_PROFILE_FUNC();

	KB_CORE_ASSERT(m_renderer_data->Stats.batch_count < 1, "Multiple batches per frame not supported!");

	m_renderer_data->render_command_buffer->Begin();

	m_renderer_data->gpu_time_query.renderer_2D_query = m_renderer_data->render_command_buffer->BeginTimestampQuery();
	const auto& render_pass = m_renderer_data->quad_pipeline->GetSpecification().render_pass;
	render::begin_render_pass(m_renderer_data->render_command_buffer, render_pass);

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

		render::render_geometry(
            m_renderer_data->render_command_buffer, 
            m_renderer_data->ui_pipeline, 
            m_renderer_data->uniform_buffer_set, 
            nullptr, 
            m_renderer_data->ui_material, 
            m_renderer_data->ui_quad_vertex_buffers[frame_index], 
            m_renderer_data->quad_index_buffer, 
            glm::mat4{ 1.0f }, 
            m_renderer_data->ui_quad_index_count
        );
		m_renderer_data->Stats.Draw_calls++;
	}

	// render text geometry
	u32 text_data_buffer_size = (u32)((uint8_t*)m_renderer_data->text_vertex_buffer_ptr - (uint8_t*)m_renderer_data->text_vertex_buffer_base_ptrs[frame_index]);
	if (text_data_buffer_size)
	{
		m_renderer_data->text_vertex_buffers[frame_index]->SetData(m_renderer_data->text_vertex_buffer_base_ptrs[frame_index], text_data_buffer_size);

		// Set Textures
		auto& textures = m_renderer_data->text_texture_atlas_slots;
		for (uint32_t i = 0; i < m_renderer_data->max_texture_slots; i++)
		{
			if (textures[i])
				m_renderer_data->text_material->Set("u_FontAtlases", textures[i], i);
			else
				m_renderer_data->text_material->Set("u_FontAtlases", m_renderer_data->white_texture, i);
		}

		render::render_geometry(
			m_renderer_data->render_command_buffer, 
			m_renderer_data->text_pipeline, 
			m_renderer_data->uniform_buffer_set, 
			nullptr, 
			m_renderer_data->text_material, 
			m_renderer_data->text_vertex_buffers[frame_index], 
			m_renderer_data->quad_index_buffer, 
			glm::mat4{ 1.0f }, 
			m_renderer_data->text_index_count
		);
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
	// Quad pipeline
	if (m_renderer_data->quad_pipeline->GetSpecification().render_pass != render_pass)
	{
        PipelineSpecification pipeline_spec = m_renderer_data->quad_pipeline->GetSpecification();
        pipeline_spec.render_pass = render_pass;
        m_renderer_data->quad_pipeline = Pipeline::Create(pipeline_spec);
    }

	// Circle pipeline
    if (m_renderer_data->circle_pipeline->GetSpecification().render_pass != render_pass)
    {
        PipelineSpecification pipeline_spec = m_renderer_data->circle_pipeline->GetSpecification();
        pipeline_spec.render_pass = render_pass;
        m_renderer_data->circle_pipeline = Pipeline::Create(pipeline_spec);
    }

    // Text Pipeline
    if (m_renderer_data->text_pipeline->GetSpecification().render_pass != render_pass)
    {
        PipelineSpecification pipeline_spec = m_renderer_data->text_pipeline->GetSpecification();
        pipeline_spec.render_pass = render_pass;
        m_renderer_data->text_pipeline = Pipeline::Create(pipeline_spec);
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

	ref<Texture2D> texture = asset::get_asset<Texture2D>(sprite_renderer_comp.Texture);
    if (!texture)
    {
        KB_CORE_ERROR("[Renderer2D]: Failed to load texture from asset id '{}'. Defaulting to white texture.", sprite_renderer_comp.Texture);
        texture = m_renderer_data->white_texture;
    }

	auto tint_color = sprite_renderer_comp.Color;
	auto tiling_factor = sprite_renderer_comp.Tiling_factor;

	draw_quad(transform, texture, tiling_factor, tint_color, static_cast<int32_t>(entity.GetHandle()));
}

// ==========================
//   Draw Quad with Texture
// ==========================

void Renderer2D::draw_quad(const glm::vec2& position, const glm::vec2& size, const ref<Texture2D>& texture, float tiling_factor, const glm::vec4& tint_color)
{
	draw_quad({ position.x, position.y, 0.0f }, size, texture, tiling_factor, tint_color);
}

void Renderer2D::draw_quad(const glm::vec3& position, const glm::vec2& size, const ref<Texture2D>& texture, float tiling_factor, const glm::vec4& tint_color)
{
	glm::mat4 transform = glm::translate(glm::mat4(1.0f), position)
		* glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });

	draw_quad(transform, texture, tiling_factor, tint_color);
}

void Renderer2D::draw_quad(const glm::mat4& transform, const ref<Texture2D>& texture, float tiling_factor, const glm::vec4& tint_color, int32_t entity_id)
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
void Renderer2D::draw_quad_from_texture_atlas(const glm::vec2& position, const glm::vec2& size, const ref<Texture2D>& texture, const glm::vec2* texture_atlas_offsets, float tiling_factor, const glm::vec4& tint_color)
{
	draw_quad_from_texture_atlas(glm::vec3{ position.x, position.y, 0.0f }, size, texture, texture_atlas_offsets, tiling_factor, tint_color);
}
void Renderer2D::draw_quad_from_texture_atlas(const glm::vec3& position, const glm::vec2& size, const ref<Texture2D>& texture, const glm::vec2* texture_atlas_offsets, float tiling_factor, const glm::vec4& tint_color)
{
	glm::mat4 transform = glm::translate(glm::mat4{ 1.0f }, position)
		* glm::rotate(glm::mat4{ 1.0f }, 0.0f, { 0.0f, 0.0f, 1.0f })
		* glm::scale(glm::mat4{ 1.0f }, { size.x, size.y, 1.0f });

	draw_quad_from_texture_atlas(transform, size, texture, texture_atlas_offsets, tiling_factor, tint_color);
}
void Renderer2D::draw_quad_from_texture_atlas(const glm::mat4& transform, const glm::vec2& size, const ref<Texture2D>& texture, const glm::vec2* texture_atlas_offsets, float tiling_factor, const glm::vec4& tint_color)
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

void Renderer2D::draw_quad_ui(const glm::vec2& position, const glm::vec2& size, const ref<Texture2D>& texture, float tiling_factor, const glm::vec4& tint_color)
{
	draw_quad_ui({ position.x, position.y, 0.0f }, size, texture, tiling_factor, tint_color);
}

void Renderer2D::draw_quad_ui(const glm::vec3& position, const glm::vec2& size, const ref<Texture2D>& texture, float tiling_factor, const glm::vec4& tint_color)
{
	glm::mat4 transform = glm::translate(glm::mat4(1.0f), position)
		* glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });

	draw_quad_ui(transform, texture, tiling_factor, tint_color);
}

void Renderer2D::draw_quad_ui(const glm::mat4& transform, const ref<Texture2D>& texture, float tiling_factor, const glm::vec4& tint_color)
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

void Renderer2D::draw_text_string(const std::string& text, const glm::vec2& position, const glm::vec2& size, const ref<render::font_asset_t>& font_asset, const glm::vec4& tint_color /* = glm::vec4{1.0f}*/)
{
	draw_text_string(text, glm::vec3{ position.x, position.y, 0.0f }, size, font_asset, tint_color);
}

void Renderer2D::draw_text_string(const std::string& text, const glm::vec3& position, const glm::vec2& size, const ref<render::font_asset_t>& font_asset, const glm::vec4& tint_color /* = glm::vec4{1.0f}*/)
{
	if (m_renderer_data->text_count + 1 > m_renderer_data->max_quads)
		end_batch();

	// check for programmer error
	KB_CORE_ASSERT(font_asset, "invalid font asset ref?");
	KB_CORE_ASSERT(!font_asset->is_flag_set(asset::asset_flag_t::Invalid), "Invalid font asset passed to render2d?");

	if (!m_renderer_data->m_font_manager.has_font_cached(font_asset))
	{
		KB_CORE_WARN("[renderer2d]: font asset was not cached in the font manager when draw command issue!");
        KB_CORE_INFO("[renderer2d]:   font asset being added to cache during render draw_text_string command!");
		m_renderer_data->m_font_manager.add_font_file_to_library(font_asset);
	}

	ref<Texture2D> font_texture_atlas = font_asset->get_texture_atlas();

	float texture_index = -1.0f;
	for (u32 i = 0; i < m_renderer_data->text_texture_atlas_slot_index; ++i)
	{
		if (*m_renderer_data->text_texture_atlas_slots[i].get() == *font_texture_atlas.get())
			texture_index = (float)i;
	}

	// insert into renderer texture atlas cache if not found
	if (texture_index == -1.0f)
	{
		texture_index = (float)m_renderer_data->text_texture_atlas_slot_index;
		m_renderer_data->text_texture_atlas_slots[m_renderer_data->text_texture_atlas_slot_index++] = font_texture_atlas;
		KB_CORE_ASSERT(m_renderer_data->text_texture_atlas_slot_index < m_renderer_data->max_texture_slots, "font texture atlas slot overflow!");
	}

	const auto& glyph_info_map = font_asset->get_glyph_rendering_map();

    glm::mat4 transform = glm::translate(glm::mat4{ 1.0f }, position)
        * glm::scale(glm::mat4{ 1.0f }, glm::vec3{ size.x, size.y, 1.0f });

	// iterate over characters and select the correct glyph bitmap
    glm::vec2 char_position = glm::vec2{ position.x, position.y };
    f32 pixel_x_scale = (static_cast<f32>(font_asset->get_font_point()) * static_cast<f32>(font_asset->get_dpi_x()) / 72.0f);
	for (char text_char : text)
	{
		const auto& glyph_data = glyph_info_map.contains(text_char) ? glyph_info_map.at(text_char) : glyph_info_map.at('?');

        const glm::vec2 render_char_position = glm::vec2{ 
            char_position.x + glyph_data.m_x_off,
            // #NOTE adding the difference between size and bearing here, since vulkan renders with -y facing up
            char_position.y - (glyph_data.m_size.y - glyph_data.m_y_off)
        };

        const f32 char_width = glyph_data.m_size.x;
        const f32 char_height = glyph_data.m_size.y;

        const glm::vec4 text_quad_coords[4] = {
            { render_char_position.x, render_char_position.y, 0.0f, 1.0f },
            { render_char_position.x + char_width, render_char_position.y, 0.0f, 1.0f },
            { render_char_position.x + char_width, render_char_position.y + char_height, 0.0f, 1.0f },
            { render_char_position.x, render_char_position.y + char_height, 0.0f, 1.0f },
        };

		// #NOTE(Sean) this is in clockwise order, since vulkan renders with +y facing down
		const glm::vec2 texture_coords[] = {
			glm::vec2{ glyph_data.m_x0, glyph_data.m_y0 },
			glm::vec2{ glyph_data.m_x1, glyph_data.m_y0 },
			glm::vec2{ glyph_data.m_x1, glyph_data.m_y1 },
			glm::vec2{ glyph_data.m_x0, glyph_data.m_y1 },
		};
			
		constexpr const size_t quad_vertex_count = 4;
		for (u32 i = 0; i < quad_vertex_count; ++i)
		{
            KB_CORE_ASSERT(texture_coords[i].x <= 1.0f && texture_coords[i].y <= 1.0f, "tex coord for font atlas out of bounds?");

			m_renderer_data->text_vertex_buffer_ptr->m_position = transform * text_quad_coords[i];
			m_renderer_data->text_vertex_buffer_ptr->m_tex_coord = texture_coords[i];
			m_renderer_data->text_vertex_buffer_ptr->m_tex_index = texture_index;
			m_renderer_data->text_vertex_buffer_ptr->m_tint_color = tint_color;
			m_renderer_data->text_vertex_buffer_ptr++;
		}

		m_renderer_data->text_count++;
		m_renderer_data->text_index_count += 6;
		m_renderer_data->Stats.Quad_count++;

        // compute position of char by offsetting the base position with the char advance offset
        char_position.x += glyph_data.m_advance;
	}
}

void Renderer2D::reset_stats()
{
	if (!m_renderer_data)
		return;

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

	m_renderer_data->text_count = 0;
	m_renderer_data->text_index_count = 0;
	m_renderer_data->text_vertex_buffer_ptr = m_renderer_data->text_vertex_buffer_base_ptrs[frame_index];

	m_renderer_data->texture_slot_index = 1;
	m_renderer_data->text_texture_atlas_slot_index = 0;

	for (size_t i = 0; i < m_renderer_data->max_texture_slots; ++i)
		if (i != 0)
			m_renderer_data->texture_slots[i] = nullptr;

	for (size_t i = 0; i < m_renderer_data->max_texture_slots; ++i)
		m_renderer_data->text_texture_atlas_slots[i] = nullptr;
}

void Renderer2D::end_batch()
{
	flush();
	start_new_batch();
}

}
