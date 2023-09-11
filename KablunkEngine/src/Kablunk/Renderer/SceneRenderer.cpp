#include "kablunkpch.h"

#include "Kablunk/Renderer/SceneRenderer.h"
#include "Kablunk/Renderer/Renderer.h"
#include "Kablunk/Renderer/RenderCommand.h"
#include "Kablunk/Renderer/RenderCommand2D.h"

#include "Kablunk/Scene/Entity.h"

#include "Platform/Vulkan/VulkanRendererAPI.h"

#include "Kablunk/UI/IPanel.h"

#include <imgui.h>

namespace Kablunk
{
	static std::vector<std::thread> s_thread_pool;

	

	SceneRenderer::SceneRenderer(const IntrusiveRef<Scene>& context, const SceneRendererSpecification& spec)
		: m_context{ context }, m_specification{ spec }
	{
		KB_CORE_ASSERT(RendererAPI::GetAPI() == RendererAPI::render_api_t::Vulkan, "SceneRenderer only supports Vulkan!");

		init();
	}

	SceneRenderer::~SceneRenderer()
	{

	}

	void SceneRenderer::init()
	{
		if (m_specification.swap_chain_target)
			m_command_buffer = RenderCommandBuffer::CreateFromSwapChain("SceneRenderer");
		else
			m_command_buffer = RenderCommandBuffer::Create(0, "SceneRenderer");


		m_bloom_texture = Texture2D::Create(ImageFormat::RGBA, 1, 1);
		m_bloom_dirt_texture = Texture2D::Create(ImageFormat::RGBA, 1, 1);

		uint32_t frames_in_flight = render::get_frames_in_flights();
		m_uniform_buffer_set = UniformBufferSet::Create(frames_in_flight);
		m_uniform_buffer_set->Create(sizeof(CameraDataUB), 0);
		m_uniform_buffer_set->Create(sizeof(glm::mat4), 1);
		m_uniform_buffer_set->Create(sizeof(PointLightUB), 2);

		m_storage_buffer_set = nullptr;//StorageBufferSet::Create(frames_in_flight);

		// Geometry
		{
			FramebufferSpecification geometry_framebuffer_spec;
			geometry_framebuffer_spec.Attachments = { ImageFormat::RGBA, ImageFormat::Depth };
			geometry_framebuffer_spec.samples = 1;
			geometry_framebuffer_spec.clear_color = { 0.1f, 0.1f, 0.1f, 1.0f };
			geometry_framebuffer_spec.debug_name = "Geometry";

			IntrusiveRef<Framebuffer> framebuffer = Framebuffer::Create(geometry_framebuffer_spec);

			PipelineSpecification pipeline_spec;
			pipeline_spec.debug_name = "GeometryPipeline";
			std::string shader_to_use = render::get_render_pipeline() == RendererPipelineDescriptor::PBR ? "Kablunk_pbr_static" : "Kablunk_diffuse_static";
 			pipeline_spec.shader = render::get_shader(shader_to_use);
			pipeline_spec.backface_culling = false;
			pipeline_spec.layout = {
				{ ShaderDataType::Float3, "a_Position" },
				{ ShaderDataType::Float3, "a_Normal" },
				{ ShaderDataType::Float3, "a_Tangent" },
				{ ShaderDataType::Float3, "a_Binormal" },
				{ ShaderDataType::Float2, "a_TexCoord" }
			};
			pipeline_spec.instance_layout = {
				{ ShaderDataType::Float4, "a_MRow0" },
				{ ShaderDataType::Float4, "a_MRow1" },
				{ ShaderDataType::Float4, "a_MRow2" },
			};

			RenderPassSpecification geo_render_pass_spec{};
			geo_render_pass_spec.target_framebuffer = framebuffer;
			geo_render_pass_spec.debug_name = "Geometry";

			pipeline_spec.render_pass = RenderPass::Create(geo_render_pass_spec);
			pipeline_spec.debug_name = "diffuse_static";

			m_geometry_pipeline = Pipeline::Create(pipeline_spec);
		}

		// Composite
		{
			FramebufferSpecification composite_framebuffer_spec;
			composite_framebuffer_spec.debug_name = "SceneComposite";
			composite_framebuffer_spec.clear_color = { 0.5f, 0.1f, 0.1f, 1.0f };
			composite_framebuffer_spec.swap_chain_target = m_specification.swap_chain_target;

			// No depth for swapchain
			if (m_specification.swap_chain_target)
				composite_framebuffer_spec.Attachments = { ImageFormat::RGBA };
			else
				composite_framebuffer_spec.Attachments = { ImageFormat::RGBA, ImageFormat::Depth };

			IntrusiveRef<Framebuffer> framebuffer = Framebuffer::Create(composite_framebuffer_spec);

			RenderPassSpecification composite_render_pass_spec;
			composite_render_pass_spec.target_framebuffer = framebuffer;
			composite_render_pass_spec.debug_name = "SceneComposite";

			IntrusiveRef<Shader> composite_shader = render::get_shader("scene_composite");

			PipelineSpecification pipeline_spec;
			pipeline_spec.layout = {
				{ ShaderDataType::Float3, "a_Position" },
				{ ShaderDataType::Float2, "a_TexCoord" }
			};
			pipeline_spec.backface_culling = false;
			pipeline_spec.shader = composite_shader;
			pipeline_spec.render_pass = RenderPass::Create(composite_render_pass_spec);
			pipeline_spec.debug_name = "SceneComposite";
			pipeline_spec.depth_write = false;
			m_composite_pipeline = Pipeline::Create(pipeline_spec);

			m_composite_material = Material::Create(composite_shader);
		}

		// external compositing
		if (!m_specification.swap_chain_target)
		{
			FramebufferSpecification external_composite_framebuffer_spec;
			external_composite_framebuffer_spec.Attachments = { ImageFormat::RGBA, ImageFormat::Depth };
			external_composite_framebuffer_spec.clear_color = { 1.0f, 0.1f, 0.1f, 1.0f };
			external_composite_framebuffer_spec.clear_on_load  = false;
			external_composite_framebuffer_spec.debug_name = "External Composite";

			// Use the color buffer from the final compositing pass, but the depth buffer from
			// the actual 3D geometry pass, in case we want to composite elements behind meshes
			// in the scene
			external_composite_framebuffer_spec.existing_images[0] = m_composite_pipeline->GetSpecification().render_pass->GetSpecification().target_framebuffer->GetImage();
			external_composite_framebuffer_spec.existing_images[1] = m_geometry_pipeline->GetSpecification().render_pass->GetSpecification().target_framebuffer->GetDepthImage();

			IntrusiveRef<Framebuffer> framebuffer = Framebuffer::Create(external_composite_framebuffer_spec);
			
			RenderPassSpecification render_pass_spec;
			render_pass_spec.target_framebuffer = framebuffer;
			render_pass_spec.debug_name = "External Composite";
			m_external_composite_render_pass = RenderPass::Create(render_pass_spec);
		}

		const size_t transform_buffer_count = 100 * 1024;
		m_transform_buffer = VertexBuffer::Create(sizeof(TransformVertexData) * transform_buffer_count);
		m_transform_vertex_data = new TransformVertexData[transform_buffer_count];

		IntrusiveRef<SceneRenderer> instance = this;
		render::submit([instance]() mutable
			{
				instance->m_resources_created = true;
			});
	}

	void SceneRenderer::set_scene(IntrusiveRef<Scene> context)
	{
		//KB_CORE_ASSERT(context, "Scene context is nullptr!");
		m_context = context;
	}

	void SceneRenderer::begin_scene(const SceneRendererCamera& camera)
	{
        KB_PROFILE_FUNC()

		KB_CORE_ASSERT(m_context, "No scene context set!");
		KB_CORE_ASSERT(!m_active, "Already active!");
		m_active = true;

		if (!m_resources_created)
			return;

		m_scene_data.camera = camera;
		m_scene_data.light_environment = m_context->m_light_environment;

		if (m_needs_resize)
		{
			m_geometry_pipeline->GetSpecification().render_pass->GetSpecification().target_framebuffer->Resize(m_viewport_width, m_viewport_height);
			m_composite_pipeline->GetSpecification().render_pass->GetSpecification().target_framebuffer->Resize(m_viewport_width, m_viewport_height);

			if (m_external_composite_render_pass)
				m_external_composite_render_pass->GetSpecification().target_framebuffer->Resize(m_viewport_width, m_viewport_height);

			m_needs_resize = false;

			if (m_specification.swap_chain_target)
				m_command_buffer = RenderCommandBuffer::CreateFromSwapChain("SceneRenderer");
		}

		// Update uniform buffers
		PointLightUB& point_light_ub_data = m_point_lights_ub;

		auto& scene_camera = m_scene_data.camera;
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
		
		IntrusiveRef<SceneRenderer> instance = this;
		render::submit([instance, camera_data]() mutable
			{
				uint32_t buffer_index = render::rt_get_current_frame_index();
				instance->m_uniform_buffer_set->Get(0, 0, buffer_index)->RT_SetData(&camera_data, sizeof(camera_data));
			}
		);

		// Set Renderer Transform
		render::submit([instance]() mutable
			{
				uint32_t buffer_index = render::rt_get_current_frame_index();
				glm::mat4 transform = glm::mat4{ 1.0f };
				instance->m_uniform_buffer_set->Get(1, 0, buffer_index)->RT_SetData(&transform, sizeof(glm::mat4));
			}
		);

		// Submit point lights uniform buffer
		const auto light_enviornment_copy = m_scene_data.light_environment;
		const std::vector<PointLight>& point_lights_vec = light_enviornment_copy.point_lights;
		point_light_ub_data.count = static_cast<uint32_t>(light_enviornment_copy.GetPointLightsSize() / sizeof(PointLight));
		std::memcpy(point_light_ub_data.point_lights, point_lights_vec.data(), light_enviornment_copy.GetPointLightsSize());
		render::submit([instance, &point_light_ub_data]() mutable
			{
				const uint32_t buffer_index = render::rt_get_current_frame_index();
				IntrusiveRef<UniformBuffer> buffer_set = instance->m_uniform_buffer_set->Get(2, 0, buffer_index);
				size_t point_light_vec_offset = 16ull;
				buffer_set->RT_SetData(&point_light_ub_data, static_cast<uint32_t>(point_light_vec_offset + sizeof(PointLight) * point_light_ub_data.count));
			}
		);
	}

	void SceneRenderer::end_scene()
	{
        KB_PROFILE_FUNC()

		if (m_use_threads)
		{
			IntrusiveRef<SceneRenderer> instance = this;
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

	void SceneRenderer::submit_mesh(IntrusiveRef<Mesh> mesh, uint32_t submesh_index, IntrusiveRef<MaterialTable> material_table, const glm::mat4& transform /*= glm::mat4{ 1.0f }*/, IntrusiveRef<Material> override_material/* = nullptr */)
	{
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

	IntrusiveRef<RenderPass> SceneRenderer::get_final_render_pass()
	{
		return m_composite_pipeline->GetSpecification().render_pass;
	}

	IntrusiveRef<Image2D> SceneRenderer::get_final_render_pass_image()
	{
        KB_PROFILE_FUNC()

		if (!m_resources_created)
			return nullptr;

		auto image = m_composite_pipeline->GetSpecification().render_pass->GetSpecification().target_framebuffer->GetImage();
		return image;
	}

	void SceneRenderer::OnImGuiRender()
	{
        KB_PROFILE_FUNC()

		ImGui::Begin("Render Statistics");

		uint32_t current_frame_index = render::rt_get_current_frame_index();
		ImGui::Text("GPU time: %.3fms", m_command_buffer->GetExecutionGPUTime(current_frame_index));
		ImGui::Text("Geometry Pass: %.3fms", m_command_buffer->GetExecutionGPUTime(current_frame_index, m_gpu_time_query_indices.geometry_pass_query));
		ImGui::Text("Composite Pass: %.3fms", m_command_buffer->GetExecutionGPUTime(current_frame_index, m_gpu_time_query_indices.composite_pass_query));

        // #TODO re-enable
		// render2d::on_imgui_render();

		ImGui::End();
	}

	void SceneRenderer::wait_for_threads()
	{
        KB_PROFILE_FUNC()

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
        KB_PROFILE_FUNC()

		m_command_buffer->Begin();
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

		m_command_buffer->End();
		m_command_buffer->Submit();

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
		render::begin_render_pass(m_command_buffer, m_composite_pipeline->GetSpecification().render_pass, true);
		render::end_render_pass(m_command_buffer);
	}

	void SceneRenderer::clear_pass(IntrusiveRef<RenderPass> render_pass, bool explicit_clear /*= false*/)
	{
		KB_CORE_INFO("Clear pass being called for renderpass '{0}'", render_pass->GetSpecification().debug_name);
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
        KB_PROFILE_FUNC()

		m_gpu_time_query_indices.geometry_pass_query = static_cast<uint32_t>(m_command_buffer->BeginTimestampQuery());
		render::begin_render_pass(m_command_buffer, m_geometry_pipeline->GetSpecification().render_pass);

		// submit transform data
		m_transform_buffer->SetData(m_transform_vertex_data, static_cast<uint32_t>(sizeof(TransformVertexData) * m_draw_list.size()), 0);
		/*render::submit([transform_buffer = m_transform_buffer, transform_data = m_transform_vertex_data, transform_count = m_draw_list.size()]() mutable
			{
				transform_buffer->RT_SetData(transform_data, static_cast<uint32_t>(sizeof(TransformVertexData) * transform_count));
			}
		);*/
		
		size_t transform_offset_ind = 0;
		for (const auto& draw_command_data : m_draw_list)
		{
			render::render_instanced_submesh(
				m_command_buffer, 
				m_geometry_pipeline, 
				m_uniform_buffer_set, 
				m_storage_buffer_set, 
				draw_command_data.Mesh, 
				draw_command_data.Submesh_index, 
				draw_command_data.Material_table, 
				m_transform_buffer, 
				static_cast<uint32_t>(transform_offset_ind++), 
				1ul
			);
		}

		render::end_render_pass(m_command_buffer);
		m_command_buffer->EndTimestampQuery(m_gpu_time_query_indices.geometry_pass_query);
	}

	void SceneRenderer::composite_pass()
	{
        KB_PROFILE_FUNC()

		m_gpu_time_query_indices.composite_pass_query = static_cast<uint32_t>(m_command_buffer->BeginTimestampQuery());
		render::begin_render_pass(m_command_buffer, m_composite_pipeline->GetSpecification().render_pass, true);

		constexpr float exposure = 1.0f; // #TODO dynamic based off camera
		constexpr bool bloom_enabled = false; // #TODO dynamic
		auto framebuffer = m_geometry_pipeline->GetSpecification().render_pass->GetSpecification().target_framebuffer;

		m_composite_material->Set("u_Uniforms.Exposure", exposure);
		if (bloom_enabled)
		{
			KB_CORE_ASSERT(false, "not implemented!");
			m_composite_material->Set("u_Uniforms.BloomIntensity", 1.0f);
			m_composite_material->Set("u_Uniforms.BloomDirtIntensity", 1.0f);
		}
		else
		{
			m_composite_material->Set("u_Uniforms.BloomIntensity", 0.0f);
			m_composite_material->Set("u_Uniforms.BloomDirtIntensity", 0.0f);
		}

		m_composite_material->Set("u_Texture", framebuffer->GetImage());
		m_composite_material->Set("u_BloomTexture", m_bloom_texture);
		m_composite_material->Set("u_BloomDirtTexture", m_bloom_dirt_texture);

		render::submit_fullscreen_quad(m_command_buffer, m_composite_pipeline, nullptr, m_composite_material);

		render::end_render_pass(m_command_buffer);
		m_command_buffer->EndTimestampQuery(m_gpu_time_query_indices.composite_pass_query);
	}

}
