#include "kablunkpch.h"

#include "Kablunk/Renderer/SceneRenderer.h"
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

SceneRenderer::SceneRenderer(const arc<Scene>& context, const SceneRendererSpecification& spec)
	: m_context{ context }, m_specification{ spec }
{
	init();
}

SceneRenderer::~SceneRenderer()
{
    delete m_point_lights_ub;
}

void SceneRenderer::init()
{
    KB_PROFILE_SCOPE;

	if (m_specification.swap_chain_target)
		m_command_buffer = backend::render_command_buffer::create_from_swap_chain("SceneRenderer");
	else
		m_command_buffer = backend::render_command_buffer::create(0, "SceneRenderer");


	m_bloom_texture = backend::texture_2d::create(backend::image_format_t::RGBA, 1, 1);
	m_bloom_dirt_texture = backend::texture_2d::create(backend::image_format_t::RGBA, 1, 1);

	uint32_t frames_in_flight = render::get_frames_in_flight();
    m_camera_uniform_buffer_set = backend::uniform_buffer_set::create(sizeof(CameraDataUB), frames_in_flight);
    m_point_lights_uniform_buffer_set = backend::uniform_buffer_set::create(sizeof(PointLightUB), frames_in_flight);

	m_storage_buffer_set = nullptr;//StorageBufferSet::Create(frames_in_flight);

    // Geometry
	{
        backend::frame_buffer_specification geometry_frame_buffer_spec{};
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
        backend::frame_buffer_specification composite_frame_buffer_spec{};
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
        const auto& white_texture = Application::Get().get_renderer_2d()->get_white_texture();
        m_composite_pass->set_input("u_BloomTexture", white_texture);
        m_composite_pass->set_input("u_BloomDirtTexture", white_texture);
        m_composite_pass->set_input("u_DepthTexture", m_geometry_pass->get_depth_output());
        KB_CORE_ASSERT(m_composite_pass->validate(), "Composite pass validation failed!");
        m_composite_pass->bake();
	}

#if 0
    render::frame_buffer_specification composite_frame_buffer{};
    composite_frame_buffer.m_attachments = { ImageFormat::RGBA, ImageFormat::Depth };
    composite_frame_buffer.m_samples = 1;
    composite_frame_buffer.m_clear_on_load = false;
    composite_frame_buffer.m_transfer = false;
    composite_frame_buffer.m_existing_images[0] = m_composite_pass->get_output_image(0);
    composite_frame_buffer.m_existing_images[1] = m_geometry_pass->get_depth_output();
    composite_frame_buffer.m_debug_name = "scene_renderer::frame_buffer::composite";

    m_external_composite_frame_buffer = render::frame_buffer::create(composite_frame_buffer);
#endif

	// external compositing
	if (!m_specification.swap_chain_target)
	{
#if 0
		frame_buffer_specification external_composite_framebuffer_spec;
		external_composite_framebuffer_spec.m_attachments = { ImageFormat::RGBA, ImageFormat::Depth };
		external_composite_framebuffer_spec.m_clear_color = { 1.0f, 0.1f, 0.1f, 1.0f };
		external_composite_framebuffer_spec.m_clear_on_load  = false;
		external_composite_framebuffer_spec.m_debug_name = "External Composite";

		// Use the color buffer from the final compositing pass, but the depth buffer from
		// the actual 3D geometry pass, in case we want to composite elements behind meshes
		// in the scene
		external_composite_framebuffer_spec.m_existing_images[0] = m_composite_pipeline->GetSpecification().render_pass->GetSpecification().target_frame_buffer->GetImage();
		external_composite_framebuffer_spec.m_existing_images[1] = m_geometry_pipeline->GetSpecification().render_pass->GetSpecification().target_frame_buffer->GetDepthImage();

		arc<Framebuffer> framebuffer = Framebuffer::Create(external_composite_framebuffer_spec);

		render_pass_specification render_pass_spec;
		render_pass_spec.target_frame_buffer = framebuffer;
		render_pass_spec.m_debug_name = "External Composite";
		m_external_composite_render_pass = RenderPass::Create(render_pass_spec);
#endif
	}

    constexpr size_t transform_buffer_count = 1024;
	m_transform_buffer = backend::vertex_buffer::create(sizeof(TransformVertexData) * transform_buffer_count);
	m_transform_vertex_data = new TransformVertexData[transform_buffer_count];

    arc<SceneRenderer> instance{ this };
	render::submit([instance]() mutable
		{
			instance->m_resources_created = true;
		});
}

void SceneRenderer::set_scene(arc<Scene> context)
{
	//KB_CORE_ASSERT(context, "Scene context is nullptr!");
	m_context = context;
}

void SceneRenderer::begin_scene(const SceneRendererCamera& camera)
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

#if 0
		if (m_external_composite_render_pass)
		{
            m_external_composite_render_pass->get_target_frame_buffer()->resize(m_viewport_width, m_viewport_height);
		}
#endif

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
	CameraDataUB camera_data = {
		view_projection,
		scene_camera.camera.GetProjection(),
		scene_camera.view_mat,
		camera_position
	};

    arc<SceneRenderer> instance{ this };
	render::submit([instance, camera_data]() mutable
		{
			instance->m_camera_uniform_buffer_set->rt_get()->rt_set_data(&camera_data, sizeof(camera_data));
		}
	);

	// Set Renderer Transform
#if 0
	render::submit([instance]() mutable
		{
			uint32_t buffer_index = render::rt_get_current_frame_index();
			glm::mat4 transform = glm::mat4{ 1.0f };
			instance->m_uniform_buffer_set->Get(1, 0, buffer_index)->RT_SetData(&transform, sizeof(glm::mat4));
		}
	);
#endif

	// Submit point lights uniform buffer
	const auto light_enviornment_copy = m_scene_data.light_environment;
	const std::vector<PointLight>& point_lights_vec = light_enviornment_copy.point_lights;

	m_point_lights_ub->count = static_cast<uint32_t>(light_enviornment_copy.GetPointLightsSize() / sizeof(PointLight));
	std::memcpy(m_point_lights_ub->point_lights, point_lights_vec.data(), light_enviornment_copy.GetPointLightsSize());

	render::submit([instance, point_lights = m_point_lights_ub]() mutable
		{
            constexpr size_t point_light_vec_offset = 16ull;
			instance->m_point_lights_uniform_buffer_set->rt_get()->rt_set_data(
                point_lights,
                static_cast<uint32_t>(point_light_vec_offset + sizeof(PointLight) * point_lights->count)
            );
		}
	);
}

void SceneRenderer::end_scene()
{
    KB_PROFILE_SCOPE;

	if (m_use_threads)
	{
        arc<SceneRenderer> instance{ this };
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

void SceneRenderer::submit_mesh(arc<Mesh> mesh, uint32_t submesh_index, arc<MaterialTable> material_table, const glm::mat4& transform /*= glm::mat4{ 1.0f }*/, arc<backend::material> override_material/* = nullptr */)
{
    KB_PROFILE_SCOPE;

	//IntrusiveRef<MeshData> mesh_data = mesh->GetMeshData();
	//uint32_t material_index = 0; // #TODO fix
	const auto& submeshes = mesh->GetMeshData()->GetSubmeshes();
	uint32_t material_index = submeshes[submesh_index].Material_index;

	m_transform_vertex_data[m_draw_list.size()].MRow[0] = {transform[0][0], transform[1][0], transform[2][0], transform[3][0]};
	m_transform_vertex_data[m_draw_list.size()].MRow[1] = {transform[0][1], transform[1][1], transform[2][1], transform[3][1]};
	m_transform_vertex_data[m_draw_list.size()].MRow[2] = {transform[0][2], transform[1][2], transform[2][2], transform[3][2]};


	// #TODO fix instancing implementation
	m_draw_list.emplace_back(DrawCommandData{ mesh, submesh_index, material_table, override_material, 1, 0, transform });
}

void SceneRenderer::set_viewport_size(uint32_t width, uint32_t height)
{
	if (m_viewport_width != width || m_viewport_height != height)
	{
		m_viewport_width = width;
		m_viewport_height = height;
		m_needs_resize = true;
	}
}

arc<backend::render_pass> SceneRenderer::get_final_render_pass()
{
	return m_composite_pass;
}

arc<backend::image_2d> SceneRenderer::get_final_render_pass_image()
{
    KB_PROFILE_SCOPE;

	if (!m_resources_created)
		return arc<backend::image_2d>{};

	auto image = m_composite_pass->get_output_image(0);
	return image;
}

void SceneRenderer::on_imgui_render(const arc<renderer_2d>& p_renderer_2d)
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

void SceneRenderer::wait_for_threads()
{
    KB_PROFILE_SCOPE;

	for (auto& thread : s_thread_pool)
		thread.join();

	s_thread_pool.clear();
}

void SceneRenderer::submit_ui_panel(ui::IPanel* panel)
{
	m_ui_panels_list.push_back(panel);
}

void SceneRenderer::flush_draw_list()
{
    KB_PROFILE_SCOPE;

	m_command_buffer->begin();
	if (m_resources_created && m_viewport_width > 0 && m_viewport_height > 0)
	{
		// do pre-render tasks
		pre_render();

		// draw 3d geometry
		geometry_pass();

		// composite and post processing pass
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

void SceneRenderer::flush_2d_draw_list()
{
    // disabled when refactoring renderer2d singleton
    // #TODO refactor
#if 0
	// 2d composite and ui pass
	if (get_final_render_pass_image())
	{
		// #TODO assert that the camera is orthographic for screen space panels

		// get camera from scene renderer data
		const glm::mat4& main_camera_proj = m_scene_data.camera.camera.GetProjection();
		const glm::mat4& main_camera_transform = m_scene_data.camera.view_mat;

		// start 2d scene rendering
		render2d::begin_scene(m_scene_data.camera.camera, main_camera_transform);
		render2d::set_target_render_pass(get_external_composite_render_pass());

		if (m_resources_created && m_viewport_width > 0 && m_viewport_height > 0)
		{
			// draw 2d elements
			two_dimensional_pass();

			// draw ui elements
			ui_pass();
		}

		render2d::end_scene();
	}
	else
		KB_CORE_ERROR("[SceneRenderer]: final composite image was not ready for 2d compositing, but renderer is not multithreaded!");
#endif

	m_entity_list.clear();
	m_ui_panels_list.clear();
}

void SceneRenderer::pre_render()
{
	// #TODO
}

void SceneRenderer::clear_pass()
{
	render::begin_render_pass(m_command_buffer, m_composite_pass, true);
	render::end_render_pass(m_command_buffer);
}

void SceneRenderer::clear_pass(arc<backend::render_pass> render_pass, bool explicit_clear /*= false*/)
{
	KB_CORE_INFO("Clear pass being called for renderpass '{0}'", render_pass->get_specification().m_debug_name);
	render::begin_render_pass(m_command_buffer, render_pass, explicit_clear);
	render::end_render_pass(m_command_buffer);
}

void SceneRenderer::ui_pass()
{
	if (m_ui_panels_list.empty())
		return;

	for (ui::IPanel* panel : m_ui_panels_list)
		panel->on_render(m_scene_data.camera);
}

void SceneRenderer::two_dimensional_pass()
{
    // disabled when refactoring renderer2d singleton
    // #TODO refactor
#if 0
	for (Entity entity : m_entity_list)
		render2d::draw_sprite(entity);
#endif

	// #TODO circles, lines, rectangles, text

}

void SceneRenderer::geometry_pass()
{
    KB_PROFILE_SCOPE;

	m_gpu_time_query_indices.geometry_pass_query = static_cast<uint32_t>(m_command_buffer->begin_timestamp_query());
	render::begin_render_pass(m_command_buffer, m_geometry_pass);

	// submit transform data
	m_transform_buffer->set_data(m_transform_vertex_data, static_cast<uint32_t>(sizeof(TransformVertexData) * m_draw_list.size()), 0);
	/*render::submit([transform_buffer = m_transform_buffer, transform_data = m_transform_vertex_data, transform_count = m_draw_list.size()]() mutable
		{
			transform_buffer->rt_set_data(transform_data, static_cast<uint32_t>(sizeof(TransformVertexData) * transform_count));
		}
	);*/

	size_t transform_offset_ind = 0;
    const auto& geometry_pipeline = m_geometry_pass->get_pipeline();
	for (const auto& draw_command_data : m_draw_list)
	{
        KB_CORE_ASSERT(false, "not implemented!");
#if 0
		render::
#endif
        Singleton<render::Renderer>::get().get_render_backend().render_instanced_submesh(
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

	render::end_render_pass(m_command_buffer);
	m_command_buffer->end_timestamp_query(m_gpu_time_query_indices.geometry_pass_query);
}

void SceneRenderer::composite_pass()
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
