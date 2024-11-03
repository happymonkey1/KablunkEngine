#include "kablunkpch.h"
#include "Kablunk/Renderer/renderer_2d.h"

#include "Kablunk/Core/Core.h"

#include "Kablunk/Asset/AssetManager.h"

#include "Kablunk/renderer/render_command.h"
#include "Kablunk/renderer/backend/uniform_buffer.h"
#include "Kablunk/renderer/Renderer.h"
#include "Kablunk/renderer/renderer_2d_utils.h"

#include "Kablunk/Scene/Components.h"

#include <glm/gtc/matrix_transform.hpp>
#include <imgui.h>

#include "Kablunk/Core/Application.h"


namespace kb::render
{ // start namespace kb::render

renderer_2d::~renderer_2d()
{
    shutdown();
}

void renderer_2d::init(renderer_2d_specification_t spec)
{
    KB_PROFILE_SCOPE;
#if 0
    KB_CORE_ASSERT(
        sizeof(QuadVertex) % 32 == 0,
        "[kb::renderer2d]: sizeof(QuadVertex)=={} != 32 or 64 bytes!",
        sizeof(QuadVertex)
    );
    KB_CORE_ASSERT(
        sizeof(text_vertex_t) % 32 == 0,
        "[kb::renderer2d]: sizeof(text_vertex_t)=={} != 32 or 64 bytes!",
        sizeof(QuadVertex)
    )
#endif
	m_renderer_data.specification = spec;

    set_swap_chain_target(m_renderer_data.specification.swap_chain_target);

	uint32_t frames_in_flight = render::get_frames_in_flight();

	// =====
	// Quads
	// =====

	m_renderer_data.quad_vertex_buffers.resize(1);
	m_renderer_data.quad_vertex_buffer_base_ptrs.resize(1);
    m_renderer_data.quad_vertex_buffer_ptrs.resize(1);

    m_renderer_data.quad_vertex_buffers[0].reserve(frames_in_flight);
    m_renderer_data.quad_vertex_buffer_base_ptrs[0].reserve(frames_in_flight);
	for (size_t i = 0; i < frames_in_flight; ++i)
	{
		m_renderer_data.quad_vertex_buffers[0].emplace_back(
            backend::vertex_buffer::create(renderer_2d_data_t::max_vertices * sizeof(QuadVertex))
        );
		m_renderer_data.quad_vertex_buffer_base_ptrs[0].emplace_back(new QuadVertex[renderer_2d_data_t::max_vertices]);
	}

	auto* quad_indices = new uint32_t[renderer_2d_data_t::max_indices];

	uint32_t offset = 0;
	for (uint32_t i = 0; i < renderer_2d_data_t::max_indices; i += 6)
	{
		quad_indices[i + uint32_t{ 0 }] = offset + uint32_t{ 0 };
		quad_indices[i + uint32_t{ 1 }] = offset + uint32_t{ 1 };
		quad_indices[i + uint32_t{ 2 }] = offset + uint32_t{ 2 };

		quad_indices[i + uint32_t{ 3 }] = offset + uint32_t{ 2 };
		quad_indices[i + uint32_t{ 4 }] = offset + uint32_t{ 3 };
		quad_indices[i + uint32_t{ 5 }] = offset + uint32_t{ 0 };

		offset += 4;
	}

	m_renderer_data.quad_index_buffer = backend::index_buffer::create(quad_indices, renderer_2d_data_t::max_indices * 4ull);
	delete[] quad_indices;

	// =======
	// Circles
	// =======

    m_renderer_data.circle_vertex_buffers.resize(1);
    m_renderer_data.circle_vertex_buffer_base_ptrs.resize(1);
    m_renderer_data.circle_vertex_buffer_ptr.resize(1);

    m_renderer_data.circle_vertex_buffers[0].reserve(frames_in_flight);
    m_renderer_data.circle_vertex_buffer_base_ptrs[0].reserve(frames_in_flight);
	for (size_t i = 0; i < frames_in_flight; ++i)
	{
        m_renderer_data.circle_vertex_buffers[0].emplace_back(backend::vertex_buffer::create(renderer_2d_data_t::max_vertices * sizeof(CircleVertex)));
        m_renderer_data.circle_vertex_buffer_base_ptrs[0].emplace_back(new CircleVertex[renderer_2d_data_t::max_vertices]);
	}

	// =====
	// Lines
	// =====

    m_renderer_data.line_vertex_buffers.resize(1);
    m_renderer_data.line_vertex_buffer_base_ptrs.resize(1);
    m_renderer_data.line_vertex_buffer_ptr.resize(1);

    m_renderer_data.line_vertex_buffers[0].reserve(frames_in_flight);
    m_renderer_data.line_vertex_buffer_base_ptrs[0].reserve(frames_in_flight);
    for (size_t i = 0; i < frames_in_flight; ++i)
    {
        m_renderer_data.line_vertex_buffers[0].emplace_back(backend::vertex_buffer::create(
            renderer_2d_data_t::max_vertices * sizeof(LineVertex))
        );
        m_renderer_data.line_vertex_buffer_base_ptrs[0].emplace_back(new LineVertex[renderer_2d_data_t::max_vertices]);
    }

	// ====
	// text
	// ====

    m_renderer_data.text_vertex_buffers.resize(1);
    m_renderer_data.text_vertex_buffer_base_ptrs.resize(1);
    m_renderer_data.text_vertex_buffer_ptr.resize(1);

    m_renderer_data.text_vertex_buffers[0].reserve(frames_in_flight);
    m_renderer_data.text_vertex_buffer_base_ptrs[0].reserve(frames_in_flight);
    for (size_t i = 0; i < frames_in_flight; ++i)
    {
        m_renderer_data.text_vertex_buffers[0].emplace_back(
            backend::vertex_buffer::create(renderer_2d_data_t::max_vertices * sizeof(text_vertex_t))
        );
        m_renderer_data.text_vertex_buffer_base_ptrs[0].emplace_back(
            new text_vertex_t[renderer_2d_data_t::max_vertices]
        );
    }

	uint32_t white_texture_data = 0xFFFFFFFF;
	m_renderer_data.white_texture = backend::texture_2d::create(backend::image_format_t::RGBA, 1, 1, &white_texture_data);

	// get references to pre-loaded shaders
	m_renderer_data.quad_shader = render::get_shader("Renderer2D_Quad");
	m_renderer_data.circle_shader = render::get_shader("Renderer2D_Circle");
	m_renderer_data.line_shader = render::get_shader("Renderer2D_Line");
	m_renderer_data.ui_shader = render::get_shader("Renderer2D_UI");
	m_renderer_data.text_shader = render::get_shader("Renderer2D_Text");

	// Set all the texture slots to zero
	//memset(s_RendererData.TextureSlots.data(), 0, s_RendererData.TextureSlots.size() * sizeof(uint32_t));

	m_renderer_data.texture_slots[0] = m_renderer_data.white_texture;

	m_renderer_data.quad_vertex_positions[0] = { -0.5f, -0.5f, 0.0f, 1.0f };
	m_renderer_data.quad_vertex_positions[1] = { 0.5f, -0.5f, 0.0f, 1.0f };
	m_renderer_data.quad_vertex_positions[2] = { 0.5f,  0.5f, 0.0f, 1.0f };
	m_renderer_data.quad_vertex_positions[3] = { -0.5f,  0.5f, 0.0f, 1.0f };

    // initialize uniform buffer sets
    m_renderer_data.m_camera_uniform_buffer_set = backend::uniform_buffer_set::create(sizeof(glm::mat4), frames_in_flight);
    //m_renderer_data.m_camera_uniform_buffer_set->Create(, 0);

	// Create framebuffer
    backend::frame_buffer_specification_t frame_buffer_spec{};
	frame_buffer_spec.m_attachments = {backend::image_format_t::RGBA, backend::image_format_t::Depth };
	frame_buffer_spec.m_samples = 1;
	frame_buffer_spec.m_clear_color_on_load = false;
    frame_buffer_spec.m_clear_color = { 51.f / 255.f, 51.f / 255.f, 51.f / 255.f, 1.0f };
	frame_buffer_spec.m_debug_name = "renderer2d::frame_buffer";
	frame_buffer_spec.m_blend_mode = backend::frame_buffer_blend_mode_t::additive;
	frame_buffer_spec.m_enable_blend = true;

    arc<backend::frame_buffer> frame_buffer = backend::frame_buffer::create(frame_buffer_spec);

    // quad render pass
    {
        backend::pipeline_specification_t quad_pipeline_spec{
            .shader = m_renderer_data.quad_shader,
            .m_target_frame_buffer = frame_buffer,
            .layout = {
                {backend::shader_data_type_t::Float3, "a_Position" },
                {backend::shader_data_type_t::Float4, "a_Color" },
                {backend::shader_data_type_t::Float2, "a_TexCoord" },
                {backend::shader_data_type_t::Float, "a_TexIndex" },
                {backend::shader_data_type_t::Float, "a_TilingFactor" },
            },
            .instance_layout = {},
            .topology = backend::primitive_topology_t::triangles,
            .backface_culling = false,
            .depth_test = true,
            .depth_write = true,
            .wireframe = false,
            .debug_name = "renderer2d::pipeline::quad"
        };

        backend::render_pass_specification render_pass_spec{
            .m_pipeline = backend::pipeline::create(quad_pipeline_spec),
            .m_debug_name = "renderer2d::render_pass::quad"
        };

        m_renderer_data.m_quad_pass = backend::render_pass::create(render_pass_spec);
        m_renderer_data.m_quad_pass->set_input("Camera", m_renderer_data.m_camera_uniform_buffer_set);
        KB_CORE_ASSERT(m_renderer_data.m_quad_pass->validate(), "quad render pass validation failed?");
        m_renderer_data.m_quad_pass->bake();
    }

	// UI
    {
        backend::pipeline_specification_t ui_pipeline_spec{
            .shader = m_renderer_data.ui_shader,
            .m_target_frame_buffer = frame_buffer,
            .layout = {
                { backend::shader_data_type_t::Float3, "a_Position" },
                { backend::shader_data_type_t::Float4, "a_Color" },
                { backend::shader_data_type_t::Float2, "a_TexCoord" },
                { backend::shader_data_type_t::Float, "a_TexIndex" },
                { backend::shader_data_type_t::Float, "a_TilingFactor" },
            },
            .instance_layout = {},
            .topology = backend::primitive_topology_t::triangles,
            .backface_culling = false,
            .depth_test = true,
            .depth_write = true,
            .wireframe = false,
            .debug_name = "renderer2d::pipeline::ui"
        };

        backend::render_pass_specification render_pass_spec{
            .m_pipeline = backend::pipeline::create(ui_pipeline_spec),
            .m_debug_name = "renderer2d::render_pass::ui"
        };

        m_renderer_data.m_ui_pass = backend::render_pass::create(render_pass_spec);
        m_renderer_data.m_ui_pass->set_input("Camera", m_renderer_data.m_camera_uniform_buffer_set);
        KB_CORE_ASSERT(m_renderer_data.m_ui_pass->validate(), "ui render pass validation failed?");
        m_renderer_data.m_ui_pass->bake();
    }

	// Circle
	{
        backend::pipeline_specification_t circle_pipeline_spec{
            .shader = m_renderer_data.circle_shader,
            .m_target_frame_buffer = frame_buffer,
            .layout = {
                { backend::shader_data_type_t::Float3, "a_WorldPosition" },
                { backend::shader_data_type_t::Float3, "a_LocalPosition" },
                { backend::shader_data_type_t::Float4, "a_Color"},
                { backend::shader_data_type_t::Float, "a_Radius" },
                { backend::shader_data_type_t::Float, "a_Thickness" },
                { backend::shader_data_type_t::Float, "a_Fade" },
                { backend::shader_data_type_t::Int, "a_EntityID" },
            },
            .instance_layout = {},
            .topology = backend::primitive_topology_t::triangles,
            .backface_culling = false,
            .depth_test = true,
            .depth_write = true,
            .wireframe = false,
            .debug_name = "render2d::pipeline::circle"
        };

        backend::render_pass_specification render_pass_spec{
            .m_pipeline = backend::pipeline::create(circle_pipeline_spec),
            .m_debug_name = "render2d::render_pass::circle"
        };

        m_renderer_data.m_circle_pass = backend::render_pass::create(render_pass_spec);
        m_renderer_data.m_circle_pass->set_input("Camera", m_renderer_data.m_camera_uniform_buffer_set);
        KB_CORE_ASSERT(m_renderer_data.m_circle_pass->validate(), "circle render pass validation failed?");
        m_renderer_data.m_circle_pass->bake();
	}

	// Line
	{
        backend::pipeline_specification_t line_pipeline_spec{
            .shader = m_renderer_data.line_shader,
            .m_target_frame_buffer = frame_buffer,
            .layout = {
                { backend::shader_data_type_t::Float3, "a_Position" },
                { backend::shader_data_type_t::Float4, "a_Color" }
            },
            .instance_layout = {},
            .topology = backend::primitive_topology_t::triangles,
            .backface_culling = false,
            .depth_test = false,
            .depth_write = false,
            .wireframe = false,
            .debug_name = "render2d::pipeline::line"
        };

        backend::render_pass_specification render_pass_spec{
            .m_pipeline = backend::pipeline::create(line_pipeline_spec),
            .m_debug_name = "render2d::render_pass::line"
        };
        m_renderer_data.m_line_pass = backend::render_pass::create(render_pass_spec);
        m_renderer_data.m_line_pass->set_input("Camera", m_renderer_data.m_camera_uniform_buffer_set);
        KB_CORE_ASSERT(m_renderer_data.m_line_pass->validate(), "line render pass validation failed?");
        m_renderer_data.m_line_pass->bake();

		uint32_t* line_indices = new uint32_t[renderer_2d_data_t::max_line_indices];
		for (uint32_t i = 0; i < renderer_2d_data_t::max_line_indices; ++i)
			line_indices[i] = i;

		m_renderer_data.line_index_buffer = backend::index_buffer::create(
            line_indices,
            renderer_2d_data_t::max_line_indices * 4ull
        );
		delete[] line_indices;
	}

	// create text pipeline
	{
        backend::pipeline_specification_t text_pipeline_spec{
            .shader = m_renderer_data.text_shader,
            .m_target_frame_buffer = frame_buffer,
            .layout = {
                { backend::shader_data_type_t::Float3, "a_Position" },
                { backend::shader_data_type_t::Float4, "a_Color" },
                { backend::shader_data_type_t::Float2, "a_TexCoord" },
                { backend::shader_data_type_t::Float, "a_TexIndex" },
            },
            .instance_layout = {},
            .topology = backend::primitive_topology_t::triangles,
            .backface_culling = false,
            .depth_test = false,
            .depth_write = false,
            .wireframe = false,
            .debug_name = "render2d::pipeline::text"
        };

        backend::render_pass_specification render_pass_spec{
            .m_pipeline = backend::pipeline::create(text_pipeline_spec),
            .m_debug_name = "render2d::render_pass::text"
        };
        m_renderer_data.m_text_pass = backend::render_pass::create(render_pass_spec);
        m_renderer_data.m_text_pass->set_input("Camera", m_renderer_data.m_camera_uniform_buffer_set);
        KB_CORE_ASSERT(m_renderer_data.m_text_pass->validate(), "text render pass validation failed?");
        m_renderer_data.m_text_pass->bake();
	}

	// create materials
	m_renderer_data.quad_material = backend::material::create(
        m_renderer_data.m_quad_pass->get_pipeline()->get_shader(),
        "render2d::material::quad_material"
    );
	m_renderer_data.circle_material = backend::material::create(
        m_renderer_data.m_circle_pass->get_pipeline()->get_shader(),
        "render2d::material::circle_material"
    );
	m_renderer_data.line_material = backend::material::create(
        m_renderer_data.m_line_pass->get_pipeline()->get_shader(),
        "render2d::material::line_material"
    );
	m_renderer_data.ui_material = backend::material::create(
        m_renderer_data.m_ui_pass->get_pipeline()->get_shader(),
        "render2d::material::ui_material"
    );
	m_renderer_data.text_material = backend::material::create(
        m_renderer_data.m_text_pass->get_pipeline()->get_shader(),
        "render2d::material::text_material"
    );

	// initialize font manager
	m_renderer_data.m_font_manager.init();
}

void renderer_2d::shutdown()
{
    KB_PROFILE_SCOPE;

	KB_CORE_INFO("Shutting down Renderer2D!");

	// free quad vertex buffers
	for (auto& frame_buffers : m_renderer_data.quad_vertex_buffer_base_ptrs)
        for (const auto buffer : frame_buffers)
		    delete[] buffer;

    m_renderer_data.quad_vertex_buffer_base_ptrs.clear();

    // free circle vertex buffers
	for (auto& frame_buffers : m_renderer_data.circle_vertex_buffer_base_ptrs)
        for (const auto buffer : frame_buffers)
		    delete[] buffer;

    m_renderer_data.circle_vertex_buffer_base_ptrs.clear();

	// free line vertex buffers
	for (auto& frame_buffers : m_renderer_data.line_vertex_buffer_base_ptrs)
        for (const auto buffer : frame_buffers)
		    delete[] buffer;

    m_renderer_data.line_vertex_buffer_base_ptrs.clear();

	// free text vertex buffers
	for (auto& frame_buffers : m_renderer_data.text_vertex_buffer_base_ptrs)
        for (const auto buffer : frame_buffers)
		    delete[] buffer;

    m_renderer_data.text_vertex_buffer_base_ptrs.clear();

    m_renderer_data.white_texture.reset();
}

auto renderer_2d::set_asset_manager(const arc<asset::AssetManager>& p_asset_manager) -> void
{
    m_asset_manager = p_asset_manager;
}

arc<backend::texture_2d> renderer_2d::get_white_texture()
{
	return m_renderer_data.white_texture;
}

void renderer_2d::begin_scene(const camera& camera, const glm::mat4& transform, bool p_explicit_clear /* = false */)
{
    KB_PROFILE_SCOPE;
    m_explicit_render_pass_clear = p_explicit_clear;

	glm::mat4 view_proj = camera.GetProjection() * transform;

    // #TODO this needs to be fixed
	arc<backend::uniform_buffer_set> uniform_buffer_set = m_renderer_data.m_camera_uniform_buffer_set;
	render::submit([uniform_buffer_set, view_proj]() mutable
		{
			uniform_buffer_set->rt_get()->rt_set_data(&view_proj, sizeof(glm::mat4));
		});

	m_renderer_data.Stats = {};

	start_new_batch();
}

void renderer_2d::begin_scene(const EditorCamera& camera, bool p_explicit_clear /* = false */)
{
    m_explicit_render_pass_clear = p_explicit_clear;
	begin_scene(camera, camera.GetViewMatrix());

	start_new_batch();
}

void renderer_2d::begin_scene(
    const glm::mat4& p_projection,
    const glm::mat4& p_transform,
    bool p_explicit_clear /* = false */
)
{
    KB_PROFILE_SCOPE;

    m_explicit_render_pass_clear = p_explicit_clear;
    glm::mat4 view_proj = p_projection * p_transform;

    // #TODO this needs to be fixed
    arc<backend::uniform_buffer_set> uniform_buffer_set = m_renderer_data.m_camera_uniform_buffer_set;
    render::submit([uniform_buffer_set, view_proj]() mutable
        {
            uniform_buffer_set->rt_get()->rt_set_data(&view_proj, sizeof(glm::mat4));
        });

    m_renderer_data.Stats = {};

    start_new_batch();
}

void renderer_2d::end_scene()
{
	flush();
}

void renderer_2d::flush()
{
    KB_PROFILE_SCOPE;

    KB_CORE_ASSERT(m_renderer_data.Stats.batch_count < 1, "Multiple batches per frame not supported!");

	m_renderer_data.render_command_buffer->begin();

    m_renderer_data.gpu_time_query.renderer_2D_query = m_renderer_data.render_command_buffer->begin_timestamp_query();

    bool clear_pass = m_explicit_render_pass_clear;

    // #TODO should this be non render thread frame?
	const uint32_t frame_index = render::rt_get_current_frame_index();

	// Quad
	// calculate data size in bytes
    for (u32 i = 0; i <= m_renderer_data.m_quad_write_index; ++i)
    {
        const auto& quad_vertex_buffer_base_ptr = m_renderer_data.quad_vertex_buffer_base_ptrs[i][frame_index];
        const auto& quad_vertex_buffer_ptr = m_renderer_data.quad_vertex_buffer_ptrs[i];

        if (const auto data_size = static_cast<u32>(reinterpret_cast<u8*>(quad_vertex_buffer_ptr) - reinterpret_cast<u8*>(quad_vertex_buffer_base_ptr)))
        {
            const u32 index_count = i == m_renderer_data.m_quad_write_index ? m_renderer_data.quad_index_count -
                (renderer_2d_data_t::max_indices * i) : renderer_2d_data_t::max_indices;

            auto& quad_vertex_buffer = m_renderer_data.quad_vertex_buffers[i][frame_index];
            quad_vertex_buffer->set_data(quad_vertex_buffer_base_ptr, data_size);

            render::begin_render_pass(
                m_renderer_data.render_command_buffer,
                m_renderer_data.m_quad_pass,
                clear_pass
            );

            // Set Textures
            auto& textures = m_renderer_data.texture_slots;
            for (uint32_t j = 0; j < renderer_2d_data_t::max_texture_slots; j++)
            {
                if (textures[j])
                    m_renderer_data.quad_material->set("u_Textures", textures[j], j);
                else
                    m_renderer_data.quad_material->set("u_Textures", m_renderer_data.white_texture, j);
            }

            const auto& quad_pipeline = m_renderer_data.m_quad_pass->get_pipeline();
            render::render_geometry(
                m_renderer_data.render_command_buffer,
                quad_pipeline,
                m_renderer_data.quad_material,
                quad_vertex_buffer,
                m_renderer_data.quad_index_buffer,
                glm::mat4{ 1.0f },
                index_count
            );
            render::end_render_pass(m_renderer_data.render_command_buffer);

            m_renderer_data.Stats.Draw_calls++;

            clear_pass = clear_pass && false;
        }
        else
        {
            render::begin_render_pass(m_renderer_data.render_command_buffer, m_renderer_data.m_quad_pass, clear_pass);
            render::end_render_pass(m_renderer_data.render_command_buffer);
            clear_pass = clear_pass && false;
        }
    }

	// Circle
    for (u32 i = 0; i <= m_renderer_data.m_circle_write_index; ++i)
    {
        const auto& circle_vertex_base_buffer_ptr = m_renderer_data.circle_vertex_buffer_base_ptrs[i][frame_index];
        const auto& circle_vertex_buffer_ptr = m_renderer_data.circle_vertex_buffer_ptr[i];

        if (const auto data_size = static_cast<u32>(reinterpret_cast<u8*>(circle_vertex_buffer_ptr) - reinterpret_cast<u8*>(circle_vertex_base_buffer_ptr)))
        {
            const u32 index_count = i == m_renderer_data.m_circle_write_index ? m_renderer_data.circle_index_count -
                (renderer_2d_data_t::max_indices * i) : renderer_2d_data_t::max_indices;

            auto& circle_vertex_buffer = m_renderer_data.circle_vertex_buffers[i][frame_index];
            circle_vertex_buffer->set_data(circle_vertex_base_buffer_ptr, data_size);

            render::begin_render_pass(m_renderer_data.render_command_buffer, m_renderer_data.m_circle_pass, clear_pass);
            const auto& circle_pipeline = m_renderer_data.m_circle_pass->get_pipeline();
            render::render_geometry(
                m_renderer_data.render_command_buffer,
                circle_pipeline,
                m_renderer_data.circle_material,
                circle_vertex_buffer,
                m_renderer_data.quad_index_buffer,
                glm::mat4{ 1.0f },
                index_count
            );
            render::end_render_pass(m_renderer_data.render_command_buffer);

            m_renderer_data.Stats.Draw_calls++;
            clear_pass = clear_pass && false;
        }
    }

	// Line
    for (u32 i = 0; i <= m_renderer_data.m_line_write_index; ++i)
    {
        const auto& line_vertex_buffer_base_ptr = m_renderer_data.line_vertex_buffer_base_ptrs[i][frame_index];
        const auto& line_vertex_buffer_ptr = m_renderer_data.line_vertex_buffer_ptr[i];

        if (const auto data_size = static_cast<u32>(reinterpret_cast<u8*>(line_vertex_buffer_ptr) - reinterpret_cast<u8*>(line_vertex_buffer_base_ptr)))
        {
            const u32 index_count = i == m_renderer_data.m_line_write_index ? m_renderer_data.line_index_count -
                (renderer_2d_data_t::max_indices * i) : renderer_2d_data_t::max_indices;

            auto& line_vertex_buffer = m_renderer_data.line_vertex_buffers[i][frame_index];
            line_vertex_buffer->set_data(line_vertex_buffer_base_ptr, data_size);

            render::begin_render_pass(m_renderer_data.render_command_buffer, m_renderer_data.m_line_pass, clear_pass);
            render::set_line_width(m_renderer_data.render_command_buffer, m_renderer_data.line_width);
            const auto& line_pipeline = m_renderer_data.m_line_pass->get_pipeline();
            render::render_geometry(
                m_renderer_data.render_command_buffer,
                line_pipeline,
                m_renderer_data.line_material,
                line_vertex_buffer,
                m_renderer_data.line_index_buffer,
                glm::mat4{ 1.0f },
                index_count
            );
            render::end_render_pass(m_renderer_data.render_command_buffer);

            m_renderer_data.Stats.Draw_calls++;
            clear_pass = clear_pass && false;
        }
    }

	// render text geometry
    for (u32 i = 0; i <= m_renderer_data.m_line_write_index; ++i)
    {
        const auto& text_vertex_buffer_base_ptr = m_renderer_data.text_vertex_buffer_base_ptrs[i][frame_index];
        const auto& text_vertex_buffer_ptr = m_renderer_data.text_vertex_buffer_ptr[i];

        if (const auto data_size = static_cast<u32>(reinterpret_cast<u8*>(text_vertex_buffer_ptr) - reinterpret_cast<u8*>(text_vertex_buffer_base_ptr)))
        {
            const u32 index_count = i == m_renderer_data.m_text_write_index ? m_renderer_data.text_index_count -
                (renderer_2d_data_t::max_indices * i) : renderer_2d_data_t::max_indices;

            auto& text_vertex_buffer = m_renderer_data.text_vertex_buffers[i][frame_index];
            text_vertex_buffer->set_data(text_vertex_buffer_base_ptr, data_size);

            // Set Textures
            auto& textures = m_renderer_data.text_texture_atlas_slots;
            for (uint32_t j = 0; j < renderer_2d_data_t::max_texture_slots; j++)
            {
                if (textures[j])
                    m_renderer_data.text_material->set("u_FontAtlases", textures[j], j);
                else
                    m_renderer_data.text_material->set("u_FontAtlases", m_renderer_data.white_texture, j);
            }

            render::begin_render_pass(m_renderer_data.render_command_buffer, m_renderer_data.m_text_pass, clear_pass);
            const auto& text_pipeline = m_renderer_data.m_text_pass->get_pipeline();
            render::render_geometry(
                m_renderer_data.render_command_buffer,
                text_pipeline,
                m_renderer_data.text_material,
                text_vertex_buffer,
                m_renderer_data.quad_index_buffer,
                glm::mat4{ 1.0f },
                index_count
            );
            render::end_render_pass(m_renderer_data.render_command_buffer);

            m_renderer_data.Stats.Draw_calls++;
            clear_pass = clear_pass && false;
        }
    }

	m_renderer_data.render_command_buffer->end_timestamp_query(m_renderer_data.gpu_time_query.renderer_2D_query);

	m_renderer_data.render_command_buffer->end();
	m_renderer_data.render_command_buffer->submit();

	m_renderer_data.Stats.batch_count++;
}

void renderer_2d::on_imgui_render() const
{
	const uint32_t current_frame_index = get_current_frame_index();
	ImGui::Text("2D Geometry Pass: %.3fms", m_renderer_data.render_command_buffer->get_execution_gpu_time(current_frame_index, static_cast<uint32_t>(m_renderer_data.gpu_time_query.renderer_2D_query)));
}

void renderer_2d::set_target_frame_buffer(const arc<backend::frame_buffer>& p_target_frame_buffer)
{
    KB_PROFILE_SCOPE;

	// Quad pipeline
	if (m_renderer_data.m_quad_pass->get_target_frame_buffer() != p_target_frame_buffer)
	{
        auto pipeline_spec = m_renderer_data.m_quad_pass->get_pipeline()->get_specification();
        pipeline_spec.m_target_frame_buffer = p_target_frame_buffer;
        auto& render_pass_spec = m_renderer_data.m_quad_pass->get_specification();
        render_pass_spec.m_pipeline = backend::pipeline::create(pipeline_spec);
    }

	// Circle pipeline
    if (m_renderer_data.m_circle_pass->get_target_frame_buffer() != p_target_frame_buffer)
    {
        auto pipeline_spec = m_renderer_data.m_circle_pass->get_pipeline()->get_specification();
        pipeline_spec.m_target_frame_buffer = p_target_frame_buffer;
        auto& render_pass_spec = m_renderer_data.m_circle_pass->get_specification();
        render_pass_spec.m_pipeline = backend::pipeline::create(pipeline_spec);
    }

    // Text Pipeline
    if (m_renderer_data.m_text_pass->get_target_frame_buffer() != p_target_frame_buffer)
    {
        auto pipeline_spec = m_renderer_data.m_text_pass->get_pipeline()->get_specification();
        pipeline_spec.m_target_frame_buffer = p_target_frame_buffer;
        auto& render_pass_spec = m_renderer_data.m_text_pass->get_specification();
        render_pass_spec.m_pipeline = backend::pipeline::create(pipeline_spec);
    }
}

auto renderer_2d::get_target_frame_buffer() const noexcept -> const arc<backend::frame_buffer>&
{
    // assumes that all render passes in `Renderer2D` targets the same frame buffer
    return m_renderer_data.m_quad_pass->get_pipeline()->get_specification().m_target_frame_buffer;
}

void renderer_2d::on_recreate_swapchain()
{
    KB_PROFILE_SCOPE;

	if (m_renderer_data.specification.swap_chain_target)
		m_renderer_data.render_command_buffer = backend::render_command_buffer::create_from_swap_chain("render_command_buffer::Renderer2D");
}

void renderer_2d::on_viewport_resize(const glm::vec2& p_viewport_dimensions)
{
    KB_PROFILE_SCOPE;

    if (m_renderer_data.specification.swap_chain_target)
        on_recreate_swapchain();
    else
    {
        // #TODO this may force recreation twice(?) depending on whether target render pass is externally managed...
        m_renderer_data.m_quad_pass->get_target_frame_buffer()->resize(
            static_cast<u32>(p_viewport_dimensions.x),
            static_cast<u32>(p_viewport_dimensions.y)
        );

        m_renderer_data.m_text_pass->get_target_frame_buffer()->resize(
            static_cast<u32>(p_viewport_dimensions.x),
            static_cast<u32>(p_viewport_dimensions.y)
        );
    }
}

void renderer_2d::set_swap_chain_target(bool p_swap_chain_target /* = true */)
{
    KB_PROFILE_SCOPE;

    m_renderer_data.specification.swap_chain_target = p_swap_chain_target;
    if (m_renderer_data.specification.swap_chain_target)
        m_renderer_data.render_command_buffer = backend::render_command_buffer::create_from_swap_chain("render_command_buffer::renderer2d::swapchain");
    else
        m_renderer_data.render_command_buffer = backend::render_command_buffer::create(0, "render_command_buffer::renderer2d");
}

// =========================
//   Draw Quad From Entity
// =========================

void renderer_2d::draw_entity(Entity entity) noexcept
{
    //KB_PROFILE_SCOPE;

    const auto transform = entity.m_scene->get_world_space_transform_matrix(entity);

    auto& sprite_renderer_comp = entity.GetComponent<SpriteRendererComponent>();

    arc<backend::texture_2d> texture = sprite_renderer_comp.Texture != asset::null_asset_id ?
                                          m_asset_manager->get_asset<backend::texture_2d>(sprite_renderer_comp.Texture) : m_renderer_data.white_texture;

    if (!texture)
    {
        KB_CORE_ERROR("[Renderer2D]: Failed to load texture from asset id '{}'. Defaulting to white texture.", sprite_renderer_comp.Texture);
        texture = m_renderer_data.white_texture;
    }

    const auto tint_color = sprite_renderer_comp.Color;
    const auto tiling_factor = sprite_renderer_comp.Tiling_factor;

    draw_quad(transform, texture, tiling_factor, tint_color, static_cast<int32_t>(entity.GetHandle()));
}

// ==========================
//   Draw Quad with Texture
// ==========================

void renderer_2d::draw_quad(const glm::mat4& transform, const arc<backend::texture_2d>& texture, float tiling_factor, const glm::vec4& tint_color, int32_t entity_id) noexcept
{
    //KB_PROFILE_SCOPE;

    // #TODO this should probably be moved out of the hotpath...
    float texture_index = 0.0f;
    {
        KB_PROFILE_SCOPE_NAMED("Find texture slot");
        for (uint32_t i = 1; i < m_renderer_data.texture_slot_index; ++i)
        {
            // Dereference shared_ptrs and compare the textures
            if (*m_renderer_data.texture_slots[i] == *texture)
                texture_index = static_cast<float>(i);
        }

        if (texture_index == 0.0f)
        {
            texture_index = static_cast<float>(m_renderer_data.texture_slot_index);
            m_renderer_data.texture_slots[m_renderer_data.texture_slot_index++] = texture;
            KB_CORE_ASSERT(m_renderer_data.texture_slot_index < m_renderer_data.max_texture_slots, "texture slot overflow!");
        }
    }

	constexpr size_t quad_vertex_count = 4;
    auto& quad_vertex_buffer_ptr = get_writeable_quad_buffer(1);
    constexpr vec2_packed k_texture_coords[] = { vec2_packed{0.0f, 0.0f}, vec2_packed{ 1.0f, 0.0f }, vec2_packed{ 1.0f, 1.0f }, vec2_packed{ 0.0f, 1.0f } };

	for (uint32_t i = 0; i < quad_vertex_count; ++i)
	{
        quad_vertex_buffer_ptr->Position = transform * m_renderer_data.quad_vertex_positions[i];
		quad_vertex_buffer_ptr->Color = tint_color;
		quad_vertex_buffer_ptr->TexCoord = k_texture_coords[i];
		quad_vertex_buffer_ptr->TexIndex = texture_index;
        quad_vertex_buffer_ptr->TilingFactor = tiling_factor;
        quad_vertex_buffer_ptr++;
	}

	m_renderer_data.quad_index_count += 6;
	m_renderer_data.quad_count++;

	m_renderer_data.Stats.Quad_count += 1;
}

void renderer_2d::draw_quad_from_texture_atlas(
    const glm::mat4& transform,
    const arc<backend::texture_2d>& texture,
    const std::array<glm::vec2, 4>& texture_atlas_offsets,
    float tiling_factor,
    const glm::vec4& tint_color
) noexcept
{
    //KB_PROFILE_SCOPE;

    // #TODO this should probably be moved out of the hotpath...
    float texture_index = 0.0f;
    {
        KB_PROFILE_SCOPE_NAMED("Find texture slot");
        for (uint32_t i = 1; i < m_renderer_data.texture_slot_index; ++i)
        {
            // Dereference shared_ptrs and compare the textures
            if (*m_renderer_data.texture_slots[i] == *texture)
                texture_index = static_cast<float>(i);
        }

        if (texture_index == 0.0f)
        {
            texture_index = static_cast<float>(m_renderer_data.texture_slot_index);
            m_renderer_data.texture_slots[m_renderer_data.texture_slot_index++] = texture;
            KB_CORE_ASSERT(
                m_renderer_data.texture_slot_index < m_renderer_data.max_texture_slots,
                "texture slot overflow!"
            );
        }
    }

	constexpr size_t quad_vertex_count = 4;
    auto& quad_vertex_buffer_ptr = get_writeable_quad_buffer(1);

	for (uint32_t i = 0; i < quad_vertex_count; ++i)
	{
		quad_vertex_buffer_ptr->Position = transform * m_renderer_data.quad_vertex_positions[i];
		quad_vertex_buffer_ptr->Color = tint_color;
		quad_vertex_buffer_ptr->TexCoord = texture_atlas_offsets[i];
		quad_vertex_buffer_ptr->TexIndex = texture_index;
		quad_vertex_buffer_ptr->TilingFactor = tiling_factor;
		quad_vertex_buffer_ptr++;
	}

	m_renderer_data.quad_index_count += 6;
	m_renderer_data.quad_count++;

	m_renderer_data.Stats.Quad_count += 1;
}

void renderer_2d::draw_quad_from_texture_atlas_no_mat(
    const glm::vec4& position,
    const glm::vec2& size,
    const arc<backend::texture_2d>& texture,
    const std::array<glm::vec2, 4>& texture_atlas_offsets,
    float tiling_factor,
    const glm::vec4& tint_color
) noexcept
{
    KB_CORE_ASSERT(false, "DO NOT USE, DOES NOT WORK");
    //KB_PROFILE_SCOPE;

    const auto texture_index = submit_texture(texture);

    constexpr size_t quad_vertex_count = 4;
    auto& quad_vertex_buffer_ptr = get_writeable_quad_buffer(1);

    for (uint32_t i = 0; i < quad_vertex_count; ++i)
    {
        quad_vertex_buffer_ptr->Position = position * m_renderer_data.quad_vertex_positions[i];
        quad_vertex_buffer_ptr->Color = tint_color;
        quad_vertex_buffer_ptr->TexCoord = texture_atlas_offsets[i];
        quad_vertex_buffer_ptr->TexIndex = static_cast<f32>(texture_index);
        quad_vertex_buffer_ptr->TilingFactor = tiling_factor;
        quad_vertex_buffer_ptr++;
    }
    m_renderer_data.quad_index_count += 6;
    m_renderer_data.quad_count++;

    m_renderer_data.Stats.Quad_count += 1;
}

void renderer_2d::draw_circle(const glm::mat4& transform, const glm::vec4& color, float radius /*= 0.5f*/, float thickness /*= 1.0f*/, float fade /*= 0.005f*/, int32_t entity_id /*= -1*/) noexcept
{
    //KB_PROFILE_SCOPE;

    auto& circle_vertex_buffer_ptr = get_writeable_circle_buffer(1);

	for (const auto& quad_vertex_position : m_renderer_data.quad_vertex_positions)
    {
		circle_vertex_buffer_ptr->WorldPosition = transform * quad_vertex_position;
		circle_vertex_buffer_ptr->LocalPosition = quad_vertex_position * 2.0f;
		circle_vertex_buffer_ptr->Color = color;
		circle_vertex_buffer_ptr->Radius = radius;
		circle_vertex_buffer_ptr->Thickness = thickness;
		circle_vertex_buffer_ptr->Fade = fade;
		circle_vertex_buffer_ptr->EntityID = entity_id;
		circle_vertex_buffer_ptr++;
	}
	m_renderer_data.circle_index_count += 6;
	m_renderer_data.circle_count++;

	m_renderer_data.Stats.Circle_count += 1;
}

void renderer_2d::draw_line(const glm::vec3& p0, const glm::vec3& p1, const glm::vec4& color /*= glm::vec4{ 1.0f }*/) noexcept
{
    //KB_PROFILE_SCOPE;

	if (m_renderer_data.line_index_count + 2 > renderer_2d_data_t::max_line_indices)
		end_batch();

    auto& line_vertex_buffer_ptr = get_writeable_line_buffer(1);

	line_vertex_buffer_ptr->Position = p0;
	line_vertex_buffer_ptr->Color = color;
	line_vertex_buffer_ptr++;

	line_vertex_buffer_ptr->Position = p1;
	line_vertex_buffer_ptr->Color = color;
	line_vertex_buffer_ptr++;

	m_renderer_data.line_index_count += 2;
	m_renderer_data.line_count += 1;

	// #TODO statistics
}

void renderer_2d::draw_rect(const glm::vec2& position, const glm::vec2& size, float rotation /* = 0 */, const glm::vec4& color /* = glm::vec4 */) noexcept
{
    //KB_PROFILE_SCOPE;

	draw_rect(glm::vec3{ position.x, position.y, 0.0f }, size, rotation, color);
}

void renderer_2d::draw_rect(const glm::vec3& position, const glm::vec2& size, float rotation /* = 0 */, const glm::vec4& color /* = glm::vec4 */) noexcept
{
    //KB_PROFILE_SCOPE;

	const glm::mat4 transform = glm::translate(glm::mat4{ 1.0f }, position)
		* glm::rotate(glm::mat4{ 1.0f }, rotation, { 0.0f, 0.0f, 1.0f })
		* glm::scale(glm::mat4{ 1.0f }, glm::vec3{ size.x, size.y, 1.0f });

	const glm::vec3 positions[4] =
	{
		transform * m_renderer_data.quad_vertex_positions[0],
		transform * m_renderer_data.quad_vertex_positions[1],
		transform * m_renderer_data.quad_vertex_positions[2],
		transform * m_renderer_data.quad_vertex_positions[3]
	};

    auto& line_vertex_buffer_ptr = get_writeable_line_buffer(4);
	for (uint32_t i = 0; i < 4; ++i)
	{
		auto& p0 = positions[i];
		auto& p1 = positions[(i + 1) % 4];

		line_vertex_buffer_ptr->Position = p0;
		line_vertex_buffer_ptr->Color = color;
		line_vertex_buffer_ptr++;

		line_vertex_buffer_ptr->Position = p1;
		line_vertex_buffer_ptr->Color = color;
		line_vertex_buffer_ptr++;

		m_renderer_data.line_index_count += 2;
		m_renderer_data.line_count += 1;
	}
}

void renderer_2d::draw_text_string(
    const std::string& text,
    const glm::vec2& position,
    const glm::vec2& size,
    const arc<render::font>& font_asset,
    const glm::vec4& tint_color, /* = glm::vec4{1.0f}*/
    f32 p_max_width, /* = 0.f */
    f32 p_line_height_offset, /*= 0.f*/
    f32 p_kerning_offset /*= 0.f*/
) noexcept
{
    //KB_PROFILE_SCOPE;

	draw_text_string(
        text,
        glm::vec3{ position.x, position.y, 0.0f },
        size,
        font_asset,
        tint_color,
        p_max_width,
        p_line_height_offset,
        p_kerning_offset
    );
}

namespace details
{ // start namespace ::details

auto is_next_line(size_t p_index, const std::vector<i32>& p_next_lines) noexcept -> bool
{
    for (const auto line : p_next_lines)
        if (line == p_index)
            return true;

    return false;
}

} // end namespace ::details

// #TODO draw command should be done on the render thread
void renderer_2d::draw_text_string(
    const std::string& text,
    const glm::vec3& position,
    const glm::vec2& size,
    const arc<render::font>& font_asset,
    const glm::vec4& tint_color, /* = glm::vec4{1.0f}*/
    f32 p_max_width, /*= 0.f*/
    f32 p_line_height_offset, /*= 0.f*/
    f32 p_kerning_offset /*= 0.f*/
) noexcept
{
    //KB_PROFILE_SCOPE;

	// check for programmer error
	KB_CORE_ASSERT(font_asset, "invalid font asset arc?");
	KB_CORE_ASSERT(!font_asset->is_flag_set(asset::asset_flag_t::Invalid), "Invalid font asset passed to render2d?");

	const auto& font_texture_atlas = font_asset->get_font_atlas();

	float texture_index = -1.0f;
	for (u32 i = 0; i < m_renderer_data.text_texture_atlas_slot_index; ++i)
	{
		if (m_renderer_data.text_texture_atlas_slots[i]->get_hash() == font_texture_atlas->get_hash())
        {
            texture_index = static_cast<float>(i);
            break;
        }
    }

	if (texture_index == -1.0f)
	{
		texture_index = static_cast<float>(m_renderer_data.text_texture_atlas_slot_index);
		m_renderer_data.text_texture_atlas_slots[m_renderer_data.text_texture_atlas_slot_index++] = font_texture_atlas;
		KB_CORE_ASSERT(m_renderer_data.text_texture_atlas_slot_index < m_renderer_data.max_texture_slots, "font texture atlas slot overflow!");
	}

    // #TODO hopefully c++23 has officially supported method that isn't deprecated...
    const std::u32string utf32_text = render::to_utf32_str(text);

    const auto& font_geometry = font_asset->get_msdf_metrics()->m_font_geometry;
    const auto& metrics = font_geometry.getMetrics();
    const f64 fs_scale = 1 / (metrics.ascenderY - metrics.descenderY);

    // compute font metrics for rendering
    // #TODO: refactor, this is slow af
    std::vector<i32> next_lines{};
    // don't waste cycles if we do not set a max width
    if (p_max_width != 0.f)
	{
        f64 x = 0.;
        f64 y = -fs_scale * metrics.ascenderY;
        i32 last_space = -1;
        for (size_t i = 0; i < utf32_text.size(); ++i)
        {
            const auto c = utf32_text[i];
            if (c == '\n')
            {
                x = 0;
                y -= fs_scale * metrics.lineHeight + p_line_height_offset;
                continue;
            }
            if (c == '\r')
                continue;

            const auto glyph = font_geometry.getGlyph(c);
            if (!glyph)
                continue;

            if (c != ' ')
            {
                double pl, pb, pr, pt;
                glyph->getQuadPlaneBounds(pl, pb, pr, pt);
                glm::vec2 quad_min{ static_cast<f32>(pl), static_cast<f32>(pb) };
                glm::vec2 quad_max{ static_cast<f32>(pr), static_cast<f32>(pt) };

                quad_min *= fs_scale;
                quad_max *= fs_scale;
                quad_min += glm::vec2{ x, y };
                quad_max += glm::vec2{ x, y };

                if (quad_max.x > p_max_width && last_space != -1)
                {
                    i = last_space;
                    next_lines.emplace_back(last_space);
                    last_space = -1;
                    x = 0;
                    y -= fs_scale * metrics.lineHeight + p_line_height_offset;
                }
            }
            else
                last_space = i;

            f64 advance = glyph->getAdvance();
            font_geometry.getAdvance(advance, c, utf32_text[i + 1]);
            x += fs_scale * advance + p_kerning_offset;
        }
	}

    const glm::mat4 transform = glm::translate(glm::mat4{ 1.0f }, position)
        * glm::scale(glm::mat4{ 1.0f }, glm::vec3{ size.x, size.y, size.x });

	{
        f64 x = 0.;
        f64 y = 0.;
        for (size_t i = 0; i < utf32_text.size(); ++i)
        {
            auto c = utf32_text[i];
            if (c == '\n' || details::is_next_line(i, next_lines))
            {
                x = 0;
                y -= fs_scale * metrics.lineHeight + p_line_height_offset;
                continue;
            }

            auto glyph = font_geometry.getGlyph(c);
            if (!glyph)
            {
                KB_CORE_WARN("[Renderer2D]: Trying to render text string, found invalid glyph for character");
                continue;
            }

            double l, b, r, t;
            glyph->getQuadAtlasBounds(l, b, r, t);

            double pl, pb, pr, pt;
            glyph->getQuadPlaneBounds(pl, pb, pr, pt);

            pl *= fs_scale;
            pl += x;
            pb *= fs_scale;
            pb += y;
            pr *= fs_scale;
            pr += x;
            pt *= fs_scale;
            pt += y;

            const f64 texel_width = 1. / font_texture_atlas->get_width();
            const f64 texel_height = 1. / font_texture_atlas->get_height();
            l *= texel_width; b *= texel_height; r *= texel_width; t *= texel_height;

            auto& buffer_ptr = get_writeable_text_buffer(1);
            buffer_ptr->m_position = transform * glm::vec4(pl, pt, 0.0f, 1.0f);
            buffer_ptr->m_tint_color = tint_color;
            buffer_ptr->m_tex_coord = vec2_packed{ static_cast<f32>(l), static_cast<f32>(t) };
            buffer_ptr->m_tex_index = texture_index;
            buffer_ptr++;

            buffer_ptr->m_position = transform * glm::vec4(pr, pt, 0.0f, 1.0f);
            buffer_ptr->m_tint_color = tint_color;
            buffer_ptr->m_tex_coord = vec2_packed{ static_cast<f32>(r), static_cast<f32>(t) };
            buffer_ptr->m_tex_index = texture_index;
            buffer_ptr++;

            buffer_ptr->m_position = transform * glm::vec4(pr, pb, 0.0f, 1.0f);
            buffer_ptr->m_tint_color = tint_color;
            buffer_ptr->m_tex_coord = vec2_packed{ static_cast<f32>(r), static_cast<f32>(b) };
            buffer_ptr->m_tex_index = texture_index;
            buffer_ptr++;

            buffer_ptr->m_position = transform * glm::vec4(pl, pb, 0.0f, 1.0f);
            buffer_ptr->m_tint_color = tint_color;
            buffer_ptr->m_tex_coord = vec2_packed{ static_cast<f32>(l), static_cast<f32>(b) };
            buffer_ptr->m_tex_index = texture_index;
            buffer_ptr++;

            m_renderer_data.text_index_count += 6;

            f64 advance = glyph->getAdvance();
            font_geometry.getAdvance(advance, c, utf32_text[i + 1]);
            x += fs_scale * advance + p_kerning_offset;

            m_renderer_data.Stats.Quad_count += 1;
        }
	}

#if 0
	const auto& glyph_info_map = font_asset->get_glyph_rendering_map();

    const glm::mat4 transform = glm::translate(glm::mat4{ 1.0f }, glm::trunc(position))
        * glm::scale(glm::mat4{ 1.0f }, glm::vec3{ size.x, size.y, 1.0f });

    const f32 scale = static_cast<f32>(p_font_point) / static_cast<f32>(render::font_manager::k_load_font_point);
    auto char_position = glm::vec2{ 0, 0 };

    // iterate over characters and select the correct glyph bitmap
	for (char text_char : text)
	{
		const auto& glyph_data = glyph_info_map.contains(text_char) ? glyph_info_map.at(text_char) : glyph_info_map.at('?');

        auto render_char_position = glm::vec2{ 
            char_position.x + glm::trunc(static_cast<f32>(glyph_data.m_x_off) * scale),
            // #NOTE adding the difference between size and bearing here, since vulkan renders with -y facing up
            char_position.y - glm::trunc(static_cast<f32>(glyph_data.m_y_off) * scale)
        };

        const f32 char_width = glm::trunc(glyph_data.m_size.x * scale);
        const f32 char_height = glm::trunc(glyph_data.m_size.y * scale);

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

		constexpr size_t quad_vertex_count = 4;
        // #TODO probably not the most efficient to potentially get a new buffer while iterating
        auto& text_vertex_buffer_ptr = get_writeable_text_buffer(1);

		for (u32 i = 0; i < quad_vertex_count; ++i)
		{
            KB_CORE_ASSERT(texture_coords[i].x <= 1.0f && texture_coords[i].y <= 1.0f, "tex coord for font atlas out of bounds?");

			text_vertex_buffer_ptr->m_position = transform * text_quad_coords[i];
			text_vertex_buffer_ptr->m_tex_coord = texture_coords[i];
			text_vertex_buffer_ptr->m_tex_index = texture_index;
			text_vertex_buffer_ptr->m_tint_color = tint_color;
			text_vertex_buffer_ptr++;
		}

		m_renderer_data.text_count++;
		m_renderer_data.text_index_count += 6;
		m_renderer_data.Stats.Quad_count++;

        // compute position of char by offsetting the base position with the char advance offset
        char_position.x += glm::trunc(glyph_data.m_advance * scale);
	}
#endif
}

auto renderer_2d::submit_quad_data(const owning_buffer& p_quad_buffer) noexcept -> void
{
    KB_PROFILE_SCOPE;

    const size_t quad_count = p_quad_buffer.size() / (4 * sizeof(QuadVertex));
    KB_CORE_ASSERT(
        m_renderer_data.quad_count + quad_count <= m_renderer_data.max_quads,
        "[Renderer2D]: quad buffer overflow!"
    );

    if (m_renderer_data.quad_count + quad_count <= renderer_2d_data_t::max_quads)
    {
        auto& quad_vertex_buffer_ptr = m_renderer_data.quad_vertex_buffer_ptrs[m_renderer_data.m_quad_write_index];
        memcpy(quad_vertex_buffer_ptr, p_quad_buffer.get(), p_quad_buffer.size());

        const size_t quad_vertex_count = quad_count * 4;
        quad_vertex_buffer_ptr += quad_vertex_count;

        {
            // #TODO should this be non render thread frame?
            const auto frame_index = render::rt_get_current_frame_index();
            auto max_ptr = m_renderer_data.quad_vertex_buffer_base_ptrs[m_renderer_data.m_quad_write_index][frame_index] + renderer_2d_data_t::max_quads;
            KB_CORE_ASSERT(
                quad_vertex_buffer_ptr < max_ptr,
                "BUFFER OVERFLOW"
            );
        }
    }
    else
    {
        const size_t quad_count_for_existing_batch = renderer_2d_data_t::max_quads - m_renderer_data.quad_count;
        const size_t quad_count_for_new_batch = quad_count - quad_count_for_existing_batch;

        const size_t existing_buffer_copy_size = quad_count_for_existing_batch * 4ul * sizeof(QuadVertex);
        const auto& existing_quad_vertex_buffer_ptr = m_renderer_data.quad_vertex_buffer_ptrs[m_renderer_data.m_quad_write_index];
        memcpy(existing_quad_vertex_buffer_ptr, p_quad_buffer.get(), existing_buffer_copy_size);

        add_quad_buffer();

        const size_t new_buffer_copy_size = quad_count_for_new_batch * 4ul * sizeof(QuadVertex);
        const auto new_quad_vertex_buffer_ptr = m_renderer_data.quad_vertex_buffer_ptrs[m_renderer_data.m_quad_write_index];
        memcpy(
            new_quad_vertex_buffer_ptr,
            static_cast<const u8*>(p_quad_buffer.get()) + existing_buffer_copy_size,
            new_buffer_copy_size
        );
    }

    m_renderer_data.quad_count += quad_count;
    m_renderer_data.quad_index_count += 6 * quad_count;
    m_renderer_data.Stats.Quad_count += quad_count;
}

void renderer_2d::reset_stats()
{
	m_renderer_data.Stats.Draw_calls = 0;
	m_renderer_data.Stats.Quad_count = 0;
	m_renderer_data.Stats.Circle_count = 0;
}

renderer_2d_stats_t renderer_2d::get_stats() { return m_renderer_data.Stats; }

void renderer_2d::start_new_batch() noexcept
{
    KB_PROFILE_SCOPE;

    // #TODO should this be non render thread frame?
    const uint32_t frame_index = render::rt_get_current_frame_index();

	m_renderer_data.quad_count = 0;
	m_renderer_data.quad_index_count = 0;
    for (size_t i = 0; i < m_renderer_data.quad_vertex_buffer_ptrs.size(); ++i)
    {
        m_renderer_data.quad_vertex_buffer_ptrs[i] = m_renderer_data.quad_vertex_buffer_base_ptrs[i][frame_index];
    }

	m_renderer_data.circle_count = 0;
	m_renderer_data.circle_index_count = 0;
    for (size_t i = 0; i < m_renderer_data.circle_vertex_buffer_ptr.size(); ++i)
    {
        m_renderer_data.circle_vertex_buffer_ptr[i] = m_renderer_data.circle_vertex_buffer_base_ptrs[i][frame_index];
    }

	m_renderer_data.line_count = 0;
	m_renderer_data.line_index_count = 0;
    for (size_t i = 0; i < m_renderer_data.line_vertex_buffer_ptr.size(); ++i)
    {
        m_renderer_data.line_vertex_buffer_ptr[i] = m_renderer_data.line_vertex_buffer_base_ptrs[i][frame_index];
    }

	m_renderer_data.text_count = 0;
	m_renderer_data.text_index_count = 0;
    for (size_t i = 0; i < m_renderer_data.text_vertex_buffer_ptr.size(); ++i)
    {
        m_renderer_data.text_vertex_buffer_ptr[i] = m_renderer_data.text_vertex_buffer_base_ptrs[i][frame_index];
    }

	m_renderer_data.texture_slot_index = 1;
	m_renderer_data.text_texture_atlas_slot_index = 0;
	for (size_t i = 0; i < renderer_2d_data_t::max_texture_slots; ++i)
	{
		if (i != 0)
			m_renderer_data.texture_slots[i] = nullptr;
	}

	for (size_t i = 0; i < renderer_2d_data_t::max_texture_slots; ++i)
	{
		m_renderer_data.text_texture_atlas_slots[i] = nullptr;
	}
}

void renderer_2d::end_batch() noexcept
{
	flush();
	start_new_batch();
}

auto renderer_2d::get_writeable_quad_buffer(u32 p_new_quad_count /* = 0 */) noexcept -> QuadVertex*&
{
    KB_PROFILE_SCOPE;

    // #TODO should this be non render thread frame?
    const u32 frame_index = render::rt_get_current_frame_index();

    const u32 quad_write_index = (m_renderer_data.quad_count + p_new_quad_count) / renderer_2d_data_t::max_quads;
    if (quad_write_index >= m_renderer_data.quad_vertex_buffer_base_ptrs.size())
    {
        KB_PROFILE_SCOPE_NAMED("Acquiring new buffer");

        add_quad_buffer();
        m_renderer_data.quad_vertex_buffer_ptrs.emplace_back(m_renderer_data.quad_vertex_buffer_base_ptrs[quad_write_index][frame_index]);
    }
    m_renderer_data.m_quad_write_index = quad_write_index;

    return m_renderer_data.quad_vertex_buffer_ptrs[m_renderer_data.m_quad_write_index];
}

auto renderer_2d::get_writeable_circle_buffer(u32 p_new_circle_count /* = 0 */) noexcept -> CircleVertex*&
{
    KB_PROFILE_SCOPE;

    // #TODO should this be non render thread frame?
    const u32 frame_index = render::rt_get_current_frame_index();

    const u32 circle_write_index = (m_renderer_data.circle_count + p_new_circle_count) /
        renderer_2d_data_t::max_quads;
    if (circle_write_index >= m_renderer_data.circle_vertex_buffer_base_ptrs.size())
    {
        add_circle_buffer();
        m_renderer_data.circle_vertex_buffer_ptr.emplace_back(m_renderer_data.circle_vertex_buffer_base_ptrs[circle_write_index][frame_index]);
    }
    m_renderer_data.m_circle_write_index = circle_write_index;

    return m_renderer_data.circle_vertex_buffer_ptr[m_renderer_data.m_circle_write_index];
}

auto renderer_2d::get_writeable_line_buffer(u32 p_new_line_count /* = 0 */) noexcept -> LineVertex*&
{
    KB_PROFILE_SCOPE;

    // #TODO should this be non render thread frame?
    const u32 frame_index = render::rt_get_current_frame_index();

    const u32 line_write_index = (m_renderer_data.line_count + p_new_line_count) / renderer_2d_data_t::max_lines;
    if (line_write_index >= m_renderer_data.line_vertex_buffer_base_ptrs.size())
    {
        add_line_buffer();
        m_renderer_data.line_vertex_buffer_ptr.emplace_back(m_renderer_data.line_vertex_buffer_base_ptrs[line_write_index][frame_index]);
    }
    m_renderer_data.m_line_write_index = line_write_index;

    return m_renderer_data.line_vertex_buffer_ptr[m_renderer_data.m_circle_write_index];
}

auto renderer_2d::get_writeable_text_buffer(u32 p_new_text_count /* = 0 */) noexcept -> text_vertex_t*&
{
    KB_PROFILE_SCOPE;

    // #TODO should this be non render thread frame?
    const u32 frame_index = render::rt_get_current_frame_index();

    const u32 text_write_index = (m_renderer_data.line_count + p_new_text_count) / renderer_2d_data_t::max_lines;
    if (text_write_index >= m_renderer_data.text_vertex_buffer_base_ptrs.size())
    {
        add_text_buffer();
        m_renderer_data.text_vertex_buffer_ptr.emplace_back(m_renderer_data.text_vertex_buffer_base_ptrs[text_write_index][frame_index]);
    }
    m_renderer_data.m_line_write_index = text_write_index;

    return m_renderer_data.text_vertex_buffer_ptr[m_renderer_data.m_line_write_index];
}

auto renderer_2d::add_quad_buffer() noexcept -> void
{
    KB_PROFILE_SCOPE;

    // #TODO should this be non render thread frame?
    const u32 frames_in_flight = render::rt_get_current_frame_index();

    renderer_2d_data_t::vertex_per_frame_buffer& new_vertex_buffer = m_renderer_data.quad_vertex_buffers.emplace_back();
    renderer_2d_data_t::quad_per_frame_base_buffer& new_vertex_buffer_base = m_renderer_data.quad_vertex_buffer_base_ptrs.emplace_back();

    new_vertex_buffer.reserve(frames_in_flight);
    new_vertex_buffer_base.reserve(frames_in_flight);

    constexpr size_t buffer_size = renderer_2d_data_t::max_vertices * sizeof(QuadVertex);
    for (u32 i = 0; i < frames_in_flight; ++i)
    {
        new_vertex_buffer.emplace_back(backend::vertex_buffer::create(buffer_size));
        new_vertex_buffer_base.emplace_back(new QuadVertex[buffer_size]);
    }
}

auto renderer_2d::add_circle_buffer() noexcept -> void
{
    KB_PROFILE_SCOPE;

    // #TODO should this be non render thread frame?
    const u32 frames_in_flight = render::rt_get_current_frame_index();

    renderer_2d_data_t::vertex_per_frame_buffer& new_vertex_buffer = m_renderer_data.circle_vertex_buffers.emplace_back();
    renderer_2d_data_t::circle_per_frame_base_buffer& new_vertex_buffer_base = m_renderer_data.circle_vertex_buffer_base_ptrs.emplace_back();

    new_vertex_buffer.reserve(frames_in_flight);
    new_vertex_buffer_base.reserve(frames_in_flight);

    constexpr size_t buffer_size = renderer_2d_data_t::max_vertices * sizeof(CircleVertex);
    for (u32 i = 0; i < frames_in_flight; ++i)
    {
        new_vertex_buffer.emplace_back(backend::vertex_buffer::create(buffer_size));
        new_vertex_buffer_base.emplace_back(new CircleVertex[buffer_size]);
    }
}

auto renderer_2d::add_line_buffer() noexcept -> void
{
    KB_PROFILE_SCOPE;

    // #TODO should this be non render thread frame?
    const u32 frames_in_flight = render::rt_get_current_frame_index();

    renderer_2d_data_t::vertex_per_frame_buffer& new_vertex_buffer = m_renderer_data.line_vertex_buffers.emplace_back();
    renderer_2d_data_t::line_per_frame_base_buffer& new_vertex_buffer_base = m_renderer_data.line_vertex_buffer_base_ptrs.emplace_back();

    new_vertex_buffer.reserve(frames_in_flight);
    new_vertex_buffer_base.reserve(frames_in_flight);

    constexpr size_t buffer_size = renderer_2d_data_t::max_line_vertices * sizeof(CircleVertex);
    for (u32 i = 0; i < frames_in_flight; ++i)
    {
        new_vertex_buffer.emplace_back(backend::vertex_buffer::create(buffer_size));
        new_vertex_buffer_base.emplace_back(new LineVertex[buffer_size]);
    }
}

auto renderer_2d::add_text_buffer() noexcept -> void
{
    KB_PROFILE_SCOPE;

    // #TODO should this be non render thread frame?
    const u32 frames_in_flight = render::rt_get_current_frame_index();

    renderer_2d_data_t::vertex_per_frame_buffer& new_vertex_buffer = m_renderer_data.text_vertex_buffers.emplace_back();
    renderer_2d_data_t::text_per_frame_base_buffer& new_vertex_buffer_base = m_renderer_data.text_vertex_buffer_base_ptrs.emplace_back();

    new_vertex_buffer.reserve(frames_in_flight);
    new_vertex_buffer_base.reserve(frames_in_flight);

    constexpr size_t buffer_size = renderer_2d_data_t::max_vertices * sizeof(CircleVertex);
    for (u32 i = 0; i < frames_in_flight; ++i)
    {
        new_vertex_buffer.emplace_back(backend::vertex_buffer::create(buffer_size));
        new_vertex_buffer_base.emplace_back(new text_vertex_t[buffer_size]);
    }
}

} // end namespace kb::render
