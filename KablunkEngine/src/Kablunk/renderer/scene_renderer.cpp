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

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/compatibility.hpp>

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

    constexpr u32 white_texture_data = 0xFFFFFFFF;
    m_bloom_texture = get_texture_2d(create_texture(
        "bloom_texture",
        backend::texture_specification_t{
            .m_format = backend::image_format_t::RGBA,
            .m_width = 1,
            .m_height = 1,
            .m_generate_mips = false,
        },
        &white_texture_data,
        false
    ));
	m_bloom_dirt_texture = get_texture_2d(create_texture(
        "bloom_dirt_texture",
        backend::texture_specification_t{
            .m_format = backend::image_format_t::RGBA,
            .m_width = 1,
            .m_height = 1,
            .m_generate_mips = false,
        },
        &white_texture_data,
        false
    ));

	uint32_t frames_in_flight = render::get_frames_in_flight();
    m_camera_uniform_buffer_set = backend::uniform_buffer_set::create(sizeof(camera_data_ub_t), frames_in_flight);
    m_point_lights_uniform_buffer_set = backend::uniform_buffer_set::create(sizeof(point_light_ub_t), frames_in_flight);
    m_directional_light_set = backend::uniform_buffer_set::create(sizeof(directional_light_t), frames_in_flight);
    m_shadow_data_uniform_buffer_set = backend::uniform_buffer_set::create(sizeof(shadow_data_ub_t), frames_in_flight);

	m_storage_buffer_set = nullptr;//StorageBufferSet::Create(frames_in_flight);

    backend::buffer_layout vertex_buffer_layout = {
        { backend::shader_data_type_t::Float3, "a_Position" },
        { backend::shader_data_type_t::Float3, "a_Normal" },
        { backend::shader_data_type_t::Float3, "a_Tangent" },
        { backend::shader_data_type_t::Float3, "a_Binormal" },
        { backend::shader_data_type_t::Float2, "a_TexCoord" }
    };

    backend::buffer_layout instance_buffer_layout = {
        { backend::shader_data_type_t::Float4, "a_MRow0" },
        { backend::shader_data_type_t::Float4, "a_MRow1" },
        { backend::shader_data_type_t::Float4, "a_MRow2" },
    };

    // Directional shadow pass
    {
        u32 shadow_map_resolution = 4096;

        backend::image_specification_t depth_image_spec{
            .format = backend::image_format_t::DEPTH32F,
            .usage = backend::image_usage_t::Attachment,
            .width = shadow_map_resolution,
            .height = shadow_map_resolution,
            .mips = 1,
            .layers = 1, // TODO: shadow cascade count
            .deinterleaved = false,
            .m_transfer = false,
            .debug_name = "Depth"
        };
        auto dir_shadow_map_depth_image = backend::image_2d::create(depth_image_spec);
        dir_shadow_map_depth_image->invalidate();

        backend::frame_buffer_specification_t frame_buffer_spec{};
        frame_buffer_spec.m_attachments = {
            backend::image_format_t::DEPTH32F
        };
        frame_buffer_spec.m_width = shadow_map_resolution;
        frame_buffer_spec.m_height = shadow_map_resolution;
        frame_buffer_spec.m_clear_color = { 0.f, 0.f, 0.f, 0.f };
        frame_buffer_spec.m_clear_depth_on_load = true;
        frame_buffer_spec.m_no_resize = true;
        frame_buffer_spec.m_depth_clear_value = 1.0f;
        frame_buffer_spec.m_existing_image = dir_shadow_map_depth_image;

        const auto& dir_shadow_shader = get_shader(shader_library::k_directional_shadows_shader_name);
        backend::pipeline_specification_t dir_shadow_pipeline_spec{
            .shader = dir_shadow_shader,
            .m_target_frame_buffer = backend::frame_buffer::create(frame_buffer_spec),
            .layout = vertex_buffer_layout,
            .instance_layout = instance_buffer_layout,
            .topology = backend::primitive_topology_t::triangles,
            .m_depth_compare_op = backend::depth_compare_op_t::less_or_equal,
            .backface_culling = true,
            .depth_test = true,
            .depth_write = true,
            .wireframe = false,
            .debug_name = "scene_renderer::pipeline::directional_shadow_map"
        };

        backend::render_pass_specification dir_shadow_render_pass_spec{
            .m_pipeline = backend::pipeline::create(dir_shadow_pipeline_spec),
            .m_debug_name = "scene_renderer::render_pass::geometry"
        };

        m_directional_shadow_pass = backend::render_pass::create(dir_shadow_render_pass_spec);
        m_directional_shadow_pass->set_input("DirShadowData", m_shadow_data_uniform_buffer_set);
        KB_CORE_ASSERT(m_directional_shadow_pass->validate(), "[scene_renderer]: Directional shadow pass validation failed!");
        m_directional_shadow_pass->bake();

        m_dir_shadow_pass_material = backend::material::create(
            dir_shadow_shader,
            "scene_renderer::material::dir_shadow_pass"
        );
    }

    // Geometry
	{
        backend::frame_buffer_specification_t geometry_frame_buffer_spec{};
        geometry_frame_buffer_spec.m_attachments = {backend::image_format_t::RGBA, backend::image_format_t::Depth };
        geometry_frame_buffer_spec.m_samples = 1;
        geometry_frame_buffer_spec.m_clear_color = { 0.1f, 0.1f, 0.1f, 1.0f };
        geometry_frame_buffer_spec.m_depth_clear_value = 0.0f;
        geometry_frame_buffer_spec.m_debug_name = "Geometry";
        //geometry_frame_buffer_spec.m_transfer = true;
        geometry_frame_buffer_spec.m_clear_color_on_load = true;
        geometry_frame_buffer_spec.m_clear_depth_on_load = true;
        arc<backend::frame_buffer> frame_buffer = backend::frame_buffer::create(geometry_frame_buffer_spec);

        arc<backend::shader> geo_shader;
        switch (get_renderer_pipeline_type())
        {
        case renderer_pipeline_type_t::basic:
        {
            geo_shader = get_shader(shader_library::k_diffuse_static_shader_name);
            break;
        }
        case renderer_pipeline_type_t::pbr:
        {
            geo_shader = get_shader(shader_library::k_pbr_static_shader_name);
            break;
        }
        default:
            KB_CORE_ASSERT(false, "[scene_renderer]: Cannot select geometry shader for unhandled renderer pipeline type!");
        }

        backend::pipeline_specification_t pipeline_spec{
            .shader = geo_shader,
            .m_target_frame_buffer = frame_buffer,
            .layout = vertex_buffer_layout,
            .instance_layout = instance_buffer_layout,
            .topology = backend::primitive_topology_t::triangles,
            .m_depth_compare_op = backend::depth_compare_op_t::greater_or_equal,
            .backface_culling = true,
            .depth_test = true,
            .depth_write = true,
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
        m_geometry_pass->set_input("DirectionalLightData", m_directional_light_set);
        m_geometry_pass->set_input("DirShadowData", m_shadow_data_uniform_buffer_set);
        m_geometry_pass->set_input("u_ShadowMapTexture", m_directional_shadow_pass->get_depth_output());

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
	const auto light_environment_copy = m_scene_data.light_environment;
	const std::vector<point_light_t>& point_lights_vec = light_environment_copy.m_point_lights;

	m_point_lights_ub->count = static_cast<uint32_t>(point_lights_vec.size());
	std::memcpy(m_point_lights_ub->point_lights, point_lights_vec.data(), light_environment_copy.GetPointLightsSize());

	render::submit([instance, point_lights = m_point_lights_ub]() mutable
		{
            constexpr size_t k_offset = 16;
			instance->m_point_lights_uniform_buffer_set->rt_get()->rt_set_data(
                point_lights,
                static_cast<uint32_t>(k_offset + sizeof(point_light_t) * point_lights->count)
            );
		}
	);

    // Submit directional light uniform buffer
    render::submit([instance, directional_light_copy = light_environment_copy.m_directional_light]() mutable
        {
            instance->m_directional_light_set->rt_get()->rt_set_data(
                &directional_light_copy,
                sizeof(directional_light_copy)
            );
        });

    const auto dir_light_vec3_packed = m_scene_data.light_environment.m_directional_light.m_direction;
    calculate_shadow_map_data(
        camera,
        glm::vec3{ dir_light_vec3_packed.x, dir_light_vec3_packed.y, dir_light_vec3_packed.z }
    );

    // Submit directional light uniform buffer
    render::submit([instance, shadow_data_ub = m_shadow_data]() mutable
        {
            instance->m_shadow_data_uniform_buffer_set->rt_get()->rt_set_data(
                &shadow_data_ub,
                sizeof(shadow_data_ub)
            );
        });
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
    uint32_t p_sub_mesh_index,
    arc<material_table> material_table,
    const glm::mat4& transform /*= glm::mat4{ 1.0f }*/,
    arc<backend::material> override_material/* = nullptr */
)
{
    KB_PROFILE_SCOPE;

	//IntrusiveRef<MeshData> mesh_data = mesh->GetMeshData();
	//uint32_t material_index = 0; // #TODO fix
	const auto& sub_meshes = mesh->GetMeshData()->get_sub_meshes();
	uint32_t material_index = sub_meshes[p_sub_mesh_index].Material_index;

    const auto mesh_handle = mesh->get_handle();
    const mesh_transform_handle mesh_transform_handle{
        .m_mesh_handle = mesh_handle,
        .m_sub_mesh_index = p_sub_mesh_index
    };

    // Submit transform
    auto& transform_data = m_transform_map[mesh_transform_handle].m_transforms.emplace_back();
    transform_data.MRow[0] = {transform[0][0], transform[1][0], transform[2][0], transform[3][0]};
    transform_data.MRow[1] = {transform[0][1], transform[1][1], transform[2][1], transform[3][1]};
    transform_data.MRow[2] = {transform[0][2], transform[1][2], transform[2][2], transform[3][2]};

    // Submit draw command
    auto& draw_command = m_draw_list[mesh_transform_handle];
    draw_command.Mesh = mesh;
    draw_command.Submesh_index = p_sub_mesh_index;
    draw_command.Material_table = std::move(material_table);
    draw_command.Override_material = std::move(override_material);
    draw_command.Instance_count++;
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
    ImGui::Text("Shadow Pass: %.3fms", m_command_buffer->get_execution_gpu_time(current_frame_index, m_gpu_time_query_indices.m_shadow_pass_query));
	ImGui::Text("Geometry Pass: %.3fms", m_command_buffer->get_execution_gpu_time(current_frame_index, m_gpu_time_query_indices.m_geometry_pass_query));
	ImGui::Text("Composite Pass: %.3fms", m_command_buffer->get_execution_gpu_time(current_frame_index, m_gpu_time_query_indices.m_composite_pass_query));

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
        shadow_pass();
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
    m_transform_map.clear();
}

void scene_renderer::pre_render()
{
    // Submit transform data
    {
        u32 count = 0;
        for (auto& [mesh_transform_handle, transform_data] : m_transform_map)
        {
            transform_data.m_transform_offset = count * sizeof(transform_vertex_data_t);
            for (const auto& transform : transform_data.m_transforms)
            {
                m_transform_vertex_data[count] = transform;
                count++;
            }
        }

        m_transform_buffer->set_data(
            m_transform_vertex_data,
            count * static_cast<u32>(sizeof(transform_vertex_data_t))
        );
    }
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

auto scene_renderer::shadow_pass() noexcept -> void
{
    m_gpu_time_query_indices.m_shadow_pass_query = static_cast<u32>(m_command_buffer->begin_timestamp_query());
    begin_render_pass(m_command_buffer, m_directional_shadow_pass);

    const auto& dir_shadow_pass_pipeline = m_directional_shadow_pass->get_pipeline();
    for (const auto& [mesh_transform_handle, draw_command_data] : m_draw_list)
    {
        const auto& transform_data = m_transform_map[mesh_transform_handle];
        const auto transform_offset = transform_data.m_transform_offset + draw_command_data.Instance_offset *
            sizeof(transform_vertex_data_t);
        KB_CORE_ASSERT(transform_offset < std::numeric_limits<u32>::max(), "[scene_renderer]: transform offset overflow!");

        Singleton<Renderer>::get().get_render_backend()->render_instanced_sub_mesh_with_material(
            m_command_buffer,
            dir_shadow_pass_pipeline,
            draw_command_data.Mesh,
            draw_command_data.Submesh_index,
            m_dir_shadow_pass_material,
            m_transform_buffer,
            static_cast<u32>(transform_offset),
            0ull,
            draw_command_data.Instance_count
        );
    }

    end_render_pass(m_command_buffer);
    m_command_buffer->end_timestamp_query(m_gpu_time_query_indices.m_shadow_pass_query);
}

void scene_renderer::geometry_pass()
{
    KB_PROFILE_SCOPE;

	m_gpu_time_query_indices.m_geometry_pass_query = static_cast<u32>(m_command_buffer->begin_timestamp_query());
	begin_render_pass(m_command_buffer, m_geometry_pass);

    const auto& geometry_pipeline = m_geometry_pass->get_pipeline();
	for (const auto& [mesh_transform_handle, draw_command_data] : m_draw_list)
	{
        const auto& transform_data = m_transform_map[mesh_transform_handle];
        const auto transform_offset = transform_data.m_transform_offset + draw_command_data.Instance_offset *
            sizeof(transform_vertex_data_t);
        KB_CORE_ASSERT(transform_offset < std::numeric_limits<u32>::max(), "[scene_renderer]: transform offset overflow!");

	    Singleton<Renderer>::get().get_render_backend()->render_instanced_sub_mesh(
            m_command_buffer,
            geometry_pipeline,
            draw_command_data.Mesh,
            draw_command_data.Submesh_index,
            draw_command_data.Material_table,
            m_transform_buffer,
            static_cast<u32>(transform_offset),
            0ull,
            draw_command_data.Instance_count
        );
	}

	end_render_pass(m_command_buffer);
	m_command_buffer->end_timestamp_query(m_gpu_time_query_indices.m_geometry_pass_query);
}

void scene_renderer::composite_pass()
{
    KB_PROFILE_SCOPE;

	m_gpu_time_query_indices.m_composite_pass_query = static_cast<uint32_t>(m_command_buffer->begin_timestamp_query());
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
	m_command_buffer->end_timestamp_query(m_gpu_time_query_indices.m_composite_pass_query);
}

auto scene_renderer::calculate_shadow_map_data(
    const scene_renderer_camera_t& p_scene_camera,
    const glm::vec3& p_light_direction
) noexcept -> void
{
    constexpr f32 scale_to_origin = 0.0f;

    // calculate view projection matrix from directional light's perspective
#if 0
    glm::mat4 view_mat = p_scene_camera.view_mat;
    const glm::vec4 origin{ glm::vec3{ 0.0f }, 1.0f };
    view_mat[3] = glm::lerp(view_mat[3], origin, scale_to_origin);
#endif
    const auto& light_direction = m_scene_data.light_environment.m_directional_light.m_direction;

    glm::vec3 light_dir_vec3 = -glm::vec3{ light_direction.x, light_direction.y, light_direction.z };
#if 1
    glm::mat4 view_mat = glm::lookAt(
        light_dir_vec3,
        glm::vec3{ 0.f },
        glm::vec3{ 0.f, 1.f, 0.f }
    );
#else
    glm::mat4 view_mat = p_scene_camera.view_mat;
    view_mat[3] = glm::lerp(view_mat[3], glm::vec4{ 0.f, 0.f, 0.f, 1.f }, 0.f);
#endif

    auto view_projection = p_scene_camera.camera.GetUnreversedProjection() * view_mat;

    // calculate frustum ortho projection
    glm::mat4 shadow_projection;
    glm::mat4 light_view_projection;
    {
        const f32 near_clip = 0.1f; // TODO: pass in scene camera
        const f32 far_clip = 1000.f; // TODO: pass in scene camera
        const f32 clip_range = far_clip - near_clip;

        f32 min_z = near_clip;
        f32 max_z = near_clip + clip_range;

        f32 range = max_z - min_z;
        f32 ratio = max_z / min_z;

        glm::vec3 frustum_corners[8] =
        {
            glm::vec3(-1.0f,  1.0f, -1.0f),
            glm::vec3(1.0f,  1.0f, -1.0f),
            glm::vec3(1.0f, -1.0f, -1.0f),
            glm::vec3(-1.0f, -1.0f, -1.0f),
            glm::vec3(-1.0f,  1.0f,  1.0f),
            glm::vec3(1.0f,  1.0f,  1.0f),
            glm::vec3(1.0f, -1.0f,  1.0f),
            glm::vec3(-1.0f, -1.0f,  1.0f),
        };

        glm::mat4 inverse_camera = glm::inverse(view_projection);
        for (u32 i = 0; i < 8; ++i)
        {
            glm::vec4 inv_corner = inverse_camera * glm::vec4(frustum_corners[i], 1.f);
            frustum_corners[i] = inv_corner / inv_corner.w;
        }

        glm::vec3 frustum_center = glm::vec3{ 0.f };
        for (u32 i = 0; i < 8; ++i)
            frustum_center += frustum_corners[i];

        frustum_center /= 8.f;

        f32 radius = 0.f;
        for (u32 i = 0; i < 8; ++i)
        {
            f32 distance = glm::length(frustum_corners[i] - frustum_center);
            radius = glm::max(radius, distance);
        }

        radius = std::ceil(radius * 16.f) / 16.f;

        glm::vec3 max_extents = glm::vec3(radius);
        glm::vec3 min_extents = -max_extents;

        glm::vec3 light_dir = -light_dir_vec3;
        glm::mat4 light_view_mat = glm::lookAt(
            frustum_center - light_dir * -min_extents.z,
            frustum_center,
            glm::vec3{ 0.f, 0.f, 1.f }
        );
        glm::mat4 light_ortho_mat = glm::ortho(
            min_extents.x,
            max_extents.x,
            min_extents.y,
            max_extents.y,
            0.f,
            max_extents.z - min_extents.z
        );

        shadow_projection = light_ortho_mat * light_view_mat;

        float shadow_map_resolution = static_cast<float>(m_directional_shadow_pass->get_target_frame_buffer()->get_width());
        glm::vec4 shadow_origin =
            shadow_projection * glm::vec4{ 0.f, 0.f, 0.f, 1.f } *
            shadow_map_resolution / 2.f;
        glm::vec4 rounded_origin = glm::round(shadow_origin);
        glm::vec4 round_offset = rounded_origin - shadow_origin;
        round_offset.z = 0.f;
        round_offset.w = 0.f;

        light_ortho_mat[3] = round_offset;
        //light_view_projection = light_ortho_mat * light_view_mat;
        //light_view_projection = glm::ortho(-10.f, 10.f, -10.f, 10.f, 0.1f, 1000.f) * light_view_mat;
    }

    // FIXME: testing
    light_view_projection = glm::ortho(-10.f, 10.f, -10.f, 10.f, 0.1f, 1000.f) * view_mat;


    //const auto light_view_projection = p_scene_camera.camera.GetUnreversedProjection() * view_mat;

    m_shadow_data.m_view_projection = light_view_projection;
}


} // end namespace kb::render
