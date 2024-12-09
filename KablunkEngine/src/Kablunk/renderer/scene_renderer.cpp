#include "kablunkpch.h"

#include "Kablunk/Renderer/scene_renderer.h"
#include "Kablunk/Renderer/Renderer.h"
#include "Kablunk/renderer/render_command.h"
#include "Kablunk/Renderer/renderer_2d.h"

#include "Kablunk/Scene/Entity.h"

#include "kablunk/renderer/backend/vulkan/VulkanRendererAPI.h"

#include "Kablunk/UI/IPanel.h"

#include <imgui.h>

#include "Kablunk/Core/Application.h"

namespace kb::render
{ // start namespace kb::render

static std::vector<std::thread> s_thread_pool;

scene_renderer::scene_renderer(const arc<Scene>& context, const scene_renderer_specification_t& spec)
	: m_context{ context }, m_specification{ spec }
{
	init();
}

scene_renderer::~scene_renderer()
{
    delete m_point_lights_ub;
}

void scene_renderer::init()
{
    KB_PROFILE_SCOPE;

	if (m_specification.swap_chain_target)
		m_command_buffer = backend::render_command_buffer::create_from_swap_chain("SceneRenderer");
	else
		m_command_buffer = backend::render_command_buffer::create(0, "SceneRenderer");


	m_bloom_texture = backend::texture_2d::create(backend::image_format_t::RGBA, 1, 1);
	m_bloom_dirt_texture = backend::texture_2d::create(backend::image_format_t::RGBA, 1, 1);

	uint32_t frames_in_flight = render::get_frames_in_flight();
    m_camera_uniform_buffer_set = backend::uniform_buffer_set::create(sizeof(camera_data_ub_t), frames_in_flight);
    m_point_lights_uniform_buffer_set = backend::uniform_buffer_set::create(sizeof(point_light_ub_t), frames_in_flight);

	m_storage_buffer_set = nullptr;//StorageBufferSet::Create(frames_in_flight);

    // Geometry
	{
        backend::frame_buffer_specification_t geometry_frame_buffer_spec{};
        geometry_frame_buffer_spec.m_attachments = {backend::image_format_t::RGBA, backend::image_format_t::Depth };
        geometry_frame_buffer_spec.m_samples = 1;
        geometry_frame_buffer_spec.m_clear_color = { 0.1f, 0.1f, 0.1f, 1.0f };
        geometry_frame_buffer_spec.m_debug_name = "Geometry";
        //geometry_frame_buffer_spec.m_transfer = true;
        geometry_frame_buffer_spec.m_clear_color_on_load = true;
        geometry_frame_buffer_spec.m_clear_depth_on_load = true;
        arc<backend::frame_buffer> frame_buffer = backend::frame_buffer::create(geometry_frame_buffer_spec);

        backend::pipeline_specification_t pipeline_spec{
            .shader = render::get_shader("Kablunk_diffuse_static"),
            .m_target_frame_buffer = frame_buffer,
            .layout = {
                { backend::shader_data_type_t::Float3, "a_Position" },
                { backend::shader_data_type_t::Float3, "a_Normal" },
                { backend::shader_data_type_t::Float3, "a_Tangent" },
                { backend::shader_data_type_t::Float3, "a_Binormal" },
                { backend::shader_data_type_t::Float2, "a_TexCoord" }
            },
            .instance_layout = {
                { backend::shader_data_type_t::Float4, "a_MRow0" },
                { backend::shader_data_type_t::Float4, "a_MRow1" },
                { backend::shader_data_type_t::Float4, "a_MRow2" },
            },
            .topology = backend::primitive_topology_t::triangles,
            .backface_culling = false,
            .depth_test = false,
            .depth_write = false,
            .wireframe = false,
            .debug_name = "scene_renderer::pipeline::geometry"
		};

        backend::render_pass_specification geo_render_pass_spec{
            .m_pipeline = backend::pipeline::create(pipeline_spec),
            .m_debug_name = "scene_renderer::render_pass::geometry"
        };

        m_geometry_pass = backend::render_pass::create(geo_render_pass_spec);

        m_geometry_pass->set_input("Camera", m_camera_uniform_buffer_set);
        m_geometry_pass->set_input("PointLightsData", m_point_lights_uniform_buffer_set);

        KB_CORE_ASSERT(m_geometry_pass->validate(), "Geometry pass validation failed!");
        m_geometry_pass->bake();
	}

	// Composite
	{
        backend::frame_buffer_specification_t composite_frame_buffer_spec{};
        composite_frame_buffer_spec.m_attachments = {backend::image_format_t::RGBA, backend::image_format_t::Depth };
        composite_frame_buffer_spec.m_samples = 1;
        composite_frame_buffer_spec.m_clear_color_on_load = false;
        composite_frame_buffer_spec.m_clear_depth_on_load = false;
        composite_frame_buffer_spec.m_transfer = false;
        composite_frame_buffer_spec.m_clear_color = { 0.1f, 0.1, 0.1f, 1.0f };
        composite_frame_buffer_spec.m_debug_name = "scene_renderer::frame_buffer::scene_composite";

        auto composite_frame_buffer = backend::frame_buffer::create(composite_frame_buffer_spec);

		arc<backend::shader> composite_shader = render::get_shader("scene_composite");
		m_composite_material = backend::material::create(composite_shader);

        backend::pipeline_specification_t pipeline_spec{
            .shader = composite_shader,
            .m_target_frame_buffer = composite_frame_buffer,
            .layout = {
                { backend::shader_data_type_t::Float3, "a_Position" },
                { backend::shader_data_type_t::Float2, "a_TexCoord" }
            },
            .instance_layout = {},
            .topology = backend::primitive_topology_t::triangles,
            .backface_culling = false,
            .depth_test = false,
            .depth_write = false,
            .wireframe = false,
            .debug_name = "scene_renderer::pipeline::scene_composite"
        };

        backend::render_pass_specification composite_render_pass{
            .m_pipeline = backend::pipeline::create(pipeline_spec),
            .m_debug_name = "scene_renderer::render_pass::scene_composite"
        };
        m_composite_pass = backend::render_pass::create(composite_render_pass);

        m_composite_pass->set_input("u_Texture", m_geometry_pass->get_output_image(0));
        const auto& white_texture = Singleton<Renderer>::get().get_white_texture();
        m_composite_pass->set_input("u_BloomTexture", white_texture);
        m_composite_pass->set_input("u_BloomDirtTexture", white_texture);
        m_composite_pass->set_input("u_DepthTexture", m_geometry_pass->get_depth_output());
        KB_CORE_ASSERT(m_composite_pass->validate(), "Composite pass validation failed!");
        m_composite_pass->bake();
	}

    constexpr size_t transform_buffer_count = 1024;
	m_transform_buffer = backend::vertex_buffer::create(sizeof(transform_vertex_data_t) * transform_buffer_count);
	m_transform_vertex_data = new transform_vertex_data_t[transform_buffer_count];

    arc instance{ this };
	render::submit([instance]() mutable
		{
			instance->m_resources_created = true;
		});
}

void scene_renderer::set_scene(arc<Scene> context)
{
	//KB_CORE_ASSERT(context, "Scene context is nullptr!");
	m_context = context;
}

void scene_renderer::begin_scene(const scene_renderer_camera_t& camera)
{
    KB_PROFILE_SCOPE;

	KB_CORE_ASSERT(m_context, "No scene context set!");
	KB_CORE_ASSERT(!m_active, "Already active!");
	m_active = true;

	if (!m_resources_created)
		return;

	m_scene_data.camera = camera;
	m_scene_data.light_environment = m_context->m_light_environment;

	if (m_needs_resize)
	{
		m_geometry_pass->get_target_frame_buffer()->resize(m_viewport_width, m_viewport_height);
		m_composite_pass->get_target_frame_buffer()->resize(m_viewport_width, m_viewport_height);

		m_needs_resize = false;

		if (m_specification.swap_chain_target)
			m_command_buffer = backend::render_command_buffer::create_from_swap_chain("SceneRenderer");
	}

	const auto& scene_camera = m_scene_data.camera;
	const auto view_projection = scene_camera.camera.GetProjection() * scene_camera.view_mat;
	const glm::mat4 view_inverse = glm::inverse(scene_camera.view_mat);
	const glm::mat4 projection_inverse = glm::inverse(scene_camera.camera.GetProjection());
	const glm::vec3 camera_position = view_inverse[3];

	const auto inverse_view_projection = glm::inverse(view_projection);

	// Set camera uniform buffer
	camera_data_ub_t camera_data = {
		view_projection,
		scene_camera.camera.GetProjection(),
		scene_camera.view_mat,
		camera_position
	};

    arc<scene_renderer> instance{ this };
	render::submit([instance, camera_data]() mutable
		{
			instance->m_camera_uniform_buffer_set->rt_get()->rt_set_data(&camera_data, sizeof(camera_data));
		}
	);

	// Submit point lights uniform buffer
	const auto light_enviornment_copy = m_scene_data.light_environment;
	const std::vector<point_light_t>& point_lights_vec = light_enviornment_copy.point_lights;

	m_point_lights_ub->count = static_cast<uint32_t>(light_enviornment_copy.GetPointLightsSize() / sizeof(point_light_t));
	std::memcpy(m_point_lights_ub->point_lights, point_lights_vec.data(), light_enviornment_copy.GetPointLightsSize());

	render::submit([instance, point_lights = m_point_lights_ub]() mutable
		{
            constexpr size_t point_light_vec_offset = 16ull;
			instance->m_point_lights_uniform_buffer_set->rt_get()->rt_set_data(
                point_lights,
                static_cast<uint32_t>(point_light_vec_offset + sizeof(point_light_t) * point_lights->count)
            );
		}
	);
}

void scene_renderer::end_scene()
{
    KB_PROFILE_SCOPE;

	if (m_use_threads)
	{
        arc<scene_renderer> instance{ this };
		s_thread_pool.emplace_back(([instance]() mutable
			{
				instance->flush_draw_list();
				//instance->flush_2d_draw_list();
			}
		));
	}
	else
	{
		flush_draw_list();
		//flush_2d_draw_list();
	}

	m_active = false;
}

void scene_renderer::submit_mesh(
    arc<Mesh> mesh,
    uint32_t submesh_index,
    arc<material_table> material_table,
    const glm::mat4& transform /*= glm::mat4{ 1.0f }*/,
    arc<backend::material> override_material/* = nullptr */
)
{
    KB_PROFILE_SCOPE;

	//IntrusiveRef<MeshData> mesh_data = mesh->GetMeshData();
	//uint32_t material_index = 0; // #TODO fix
	const auto& sub_meshes = mesh->GetMeshData()->get_sub_meshes();
	uint32_t material_index = sub_meshes[submesh_index].Material_index;

	m_transform_vertex_data[m_draw_list.size()].MRow[0] = {transform[0][0], transform[1][0], transform[2][0], transform[3][0]};
	m_transform_vertex_data[m_draw_list.size()].MRow[1] = {transform[0][1], transform[1][1], transform[2][1], transform[3][1]};
	m_transform_vertex_data[m_draw_list.size()].MRow[2] = {transform[0][2], transform[1][2], transform[2][2], transform[3][2]};


	// #TODO fix instancing implementation
	m_draw_list.emplace_back(draw_command_data_t{ mesh, submesh_index, material_table, override_material, 1, 0, transform });
}

void scene_renderer::set_viewport_size(uint32_t width, uint32_t height)
{
	if (m_viewport_width != width || m_viewport_height != height)
	{
		m_viewport_width = width;
		m_viewport_height = height;
		m_needs_resize = true;
	}
}

arc<backend::render_pass> scene_renderer::get_final_render_pass()
{
	return m_composite_pass;
}

arc<backend::image_2d> scene_renderer::get_final_render_pass_image()
{
    KB_PROFILE_SCOPE;

	if (!m_resources_created)
		return arc<backend::image_2d>{};

	auto image = m_composite_pass->get_output_image(0);
	return image;
}

void scene_renderer::on_imgui_render(const arc<renderer_2d>& p_renderer_2d)
{
    KB_PROFILE_SCOPE;

	ImGui::Begin("Render Statistics");

	uint32_t current_frame_index = rt_get_current_frame_index();
	ImGui::Text("GPU time: %.3fms", m_command_buffer->get_execution_gpu_time(current_frame_index));
	ImGui::Text("Geometry Pass: %.3fms", m_command_buffer->get_execution_gpu_time(current_frame_index, m_gpu_time_query_indices.geometry_pass_query));
	ImGui::Text("Composite Pass: %.3fms", m_command_buffer->get_execution_gpu_time(current_frame_index, m_gpu_time_query_indices.composite_pass_query));

    p_renderer_2d->on_imgui_render();

	ImGui::End();
}

void scene_renderer::wait_for_threads()
{
    KB_PROFILE_SCOPE;

	for (auto& thread : s_thread_pool)
		thread.join();

	s_thread_pool.clear();
}

void scene_renderer::flush_draw_list()
{
    KB_PROFILE_SCOPE;

	m_command_buffer->begin();
	if (m_resources_created && m_viewport_width > 0 && m_viewport_height > 0)
	{
		// do pre-render tasks
		pre_render();

		// draw 3d geometry
		geometry_pass();

		// composite and post-processing pass
		composite_pass();
	}
	else
	{
		clear_pass();
	}

	m_command_buffer->end();
	m_command_buffer->submit();

	m_scene_data = {};
	m_draw_list = {};
}

void scene_renderer::pre_render()
{
	// #TODO
}

void scene_renderer::clear_pass()
{
	render::begin_render_pass(m_command_buffer, m_composite_pass, true);
	render::end_render_pass(m_command_buffer);
}

void scene_renderer::clear_pass(arc<backend::render_pass> render_pass, bool explicit_clear /*= false*/)
{
	KB_CORE_INFO("Clear pass being called for renderpass '{0}'", render_pass->get_specification().m_debug_name);
	render::begin_render_pass(m_command_buffer, render_pass, explicit_clear);
	render::end_render_pass(m_command_buffer);
}

void scene_renderer::geometry_pass()
{
    KB_PROFILE_SCOPE;

	m_gpu_time_query_indices.geometry_pass_query = static_cast<u32>(m_command_buffer->begin_timestamp_query());
	begin_render_pass(m_command_buffer, m_geometry_pass);

	// submit transform data
	m_transform_buffer->set_data(m_transform_vertex_data, static_cast<u32>(sizeof(transform_vertex_data_t) * m_draw_list.size()), 0);

	size_t transform_offset_ind = 0;
    const auto& geometry_pipeline = m_geometry_pass->get_pipeline();
	for (const auto& draw_command_data : m_draw_list)
	{
        Singleton<Renderer>::get().get_render_backend()->render_instanced_sub_mesh(
            m_command_buffer,
            geometry_pipeline,
            draw_command_data.Mesh,
            draw_command_data.Submesh_index,
            draw_command_data.Material_table,
            m_transform_buffer,
            static_cast<uint32_t>(transform_offset_ind++),
            0ull,
            1ul
        );
	}

	end_render_pass(m_command_buffer);
	m_command_buffer->end_timestamp_query(m_gpu_time_query_indices.geometry_pass_query);
}

void scene_renderer::composite_pass()
{
    KB_PROFILE_SCOPE;

	m_gpu_time_query_indices.composite_pass_query = static_cast<uint32_t>(m_command_buffer->begin_timestamp_query());
	render::begin_render_pass(m_command_buffer, m_composite_pass, true);

	constexpr float exposure = 1.0f; // #TODO dynamic based off camera
	constexpr bool bloom_enabled = false; // #TODO dynamic
	auto frame_buffer = m_geometry_pass->get_target_frame_buffer();

	m_composite_material->set("u_Uniforms.Exposure", exposure);
	if (bloom_enabled)
	{
		KB_CORE_ASSERT(false, "not implemented!");
		m_composite_material->set("u_Uniforms.BloomIntensity", 1.0f);
		m_composite_material->set("u_Uniforms.BloomDirtIntensity", 1.0f);
	}
	else
	{
		m_composite_material->set("u_Uniforms.BloomIntensity", 0.0f);
		m_composite_material->set("u_Uniforms.BloomDirtIntensity", 0.0f);
	}

    const float saturation = 1.0f;
    const float contrast = 1.0f;
    const float brightness = 0.0f;
    m_composite_material->set("u_Uniforms.Saturation", saturation);
    m_composite_material->set("u_Uniforms.Contrast", contrast);
    m_composite_material->set("u_Uniforms.Brightness", brightness);

	//m_composite_material->Set("u_Texture", frame_buffer->GetImage());
	//m_composite_material->Set("u_BloomTexture", m_bloom_texture);
	//m_composite_material->Set("u_BloomDirtTexture", m_bloom_dirt_texture);

    const auto& composite_pipeline = m_composite_pass->get_pipeline();
    render::submit_fullscreen_quad(
        m_command_buffer,
        composite_pipeline,
        m_composite_material
    );

	render::end_render_pass(m_command_buffer);
	m_command_buffer->end_timestamp_query(m_gpu_time_query_indices.composite_pass_query);
}

} // end namespace kb::render
