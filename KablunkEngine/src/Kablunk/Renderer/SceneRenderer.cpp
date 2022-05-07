#include "kablunkpch.h"

#include "Kablunk/Renderer/SceneRenderer.h"
#include "Kablunk/Renderer/Renderer.h"
#include "Kablunk/Renderer/Renderer2D.h"

#include "Platform/Vulkan/VulkanRendererAPI.h"

#include <imgui.h>

namespace Kablunk
{
	static std::vector<std::thread> s_thread_pool;

	

	SceneRenderer::SceneRenderer(const IntrusiveRef<Scene>& context, const SceneRendererSpecification& spec)
		: m_context{ context }, m_specification{ spec }
	{
		KB_CORE_ASSERT(RendererAPI::GetAPI() == RendererAPI::RenderAPI_t::Vulkan, "SceneRenderer only supports Vulkan!");

		Init();
	}

	SceneRenderer::~SceneRenderer()
	{

	}

	void SceneRenderer::Init()
	{
		if (m_specification.swap_chain_target)
			m_command_buffer = RenderCommandBuffer::CreateFromSwapChain("SceneRenderer");
		else
			m_command_buffer = RenderCommandBuffer::Create(0, "SceneRenderer");


		m_bloom_texture = Texture2D::Create(ImageFormat::RGBA, 1, 1);
		m_bloom_dirt_texture = Texture2D::Create(ImageFormat::RGBA, 1, 1);

		uint32_t frames_in_flight = Renderer::GetConfig().frames_in_flight;
		m_uniform_buffer_set = UniformBufferSet::Create(frames_in_flight);
		m_uniform_buffer_set->Create(sizeof(CameraDataUB), 0);

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
			pipeline_spec.shader = Renderer::GetShaderLibrary()->Get("Kablunk_diffuse_static");
			pipeline_spec.backface_culling = false;
			pipeline_spec.layout = {
				{ ShaderDataType::Float3, "a_Position" },
				{ ShaderDataType::Float3, "a_Normal" },
				{ ShaderDataType::Float3, "a_Tangent" },
				{ ShaderDataType::Float3, "a_Binormal" },
				{ ShaderDataType::Float2, "a_TexCoord" },
				{ ShaderDataType::Int, "a_EntityID" }
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

			IntrusiveRef<Shader> composite_shader = Renderer::GetShaderLibrary()->Get("scene_composite");

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

		IntrusiveRef<SceneRenderer> instance = this;
		RenderCommand::Submit([instance]() mutable
			{
				instance->m_resources_created = true;
			});
	}

	void SceneRenderer::SetScene(IntrusiveRef<Scene> context)
	{
		//KB_CORE_ASSERT(context, "Scene context is nullptr!");
		m_context = context;
	}

	void SceneRenderer::BeginScene(const SceneRendererCamera& camera)
	{
		KB_CORE_ASSERT(m_context, "No scene context set!");
		KB_CORE_ASSERT(!m_active, "Already active!");
		m_active = true;

		if (!m_resources_created)
			return;

		m_scene_data.camera = camera;

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

		auto& scene_camera = m_scene_data.camera;
		const auto view_projection = scene_camera.camera.GetProjection() * scene_camera.view_mat;
		const glm::vec3 camera_position = glm::inverse(scene_camera.view_mat)[3];

		const auto inverse_view_projection = glm::inverse(view_projection);
		
		// #TODO set camera uniform buffer
		CameraDataUB camera_data = { scene_camera.camera.GetProjection() * scene_camera.view_mat };
		IntrusiveRef<SceneRenderer> instance = this;
		RenderCommand::Submit([instance, camera_data]() mutable
			{
				uint32_t buffer_index = Renderer::GetCurrentFrameIndex();
				instance->m_uniform_buffer_set->Get(0, 0, buffer_index)->RT_SetData(&camera_data, sizeof(camera_data));
			});
	}

	void SceneRenderer::EndScene()
	{
		if (m_use_threads)
		{
			IntrusiveRef<SceneRenderer> instance = this;
			s_thread_pool.emplace_back(([instance]() mutable
				{
					instance->FlushDrawList();
				}
			));
		}
		else
			FlushDrawList();

		m_active = false;
	}

	void SceneRenderer::SetViewportSize(uint32_t width, uint32_t height)
	{
		if (m_viewport_width != width || m_viewport_height!= height)
		{
			m_viewport_width = width;
			m_viewport_height = height;
			m_needs_resize = true;
		}
	}

	IntrusiveRef<RenderPass> SceneRenderer::GetFinalRenderPass()
	{
		return m_composite_pipeline->GetSpecification().render_pass;
	}

	IntrusiveRef<Image2D> SceneRenderer::GetFinalPassImage()
	{
		if (!m_resources_created)
			return nullptr;

		auto image = m_composite_pipeline->GetSpecification().render_pass->GetSpecification().target_framebuffer->GetImage();
		return image;
	}

	void SceneRenderer::OnImGuiRender()
	{
		ImGui::Begin("Render Statistics");

		uint32_t current_frame_index = Renderer::GetCurrentFrameIndex();
		ImGui::Text("GPU time: %.3fms", m_command_buffer->GetExecutionGPUTime(current_frame_index));
		ImGui::Text("Geometry Pass: %.3fms", m_command_buffer->GetExecutionGPUTime(current_frame_index, m_gpu_time_query_indices.geometry_pass_query));
		ImGui::Text("Composite Pass: %.3fms", m_command_buffer->GetExecutionGPUTime(current_frame_index, m_gpu_time_query_indices.composite_pass_query));

		Renderer2D::OnImGuiRender();

		ImGui::End();
	}

	void SceneRenderer::WaitForThreads()
	{
		for (auto& thread : s_thread_pool)
			thread.join();

		s_thread_pool.clear();
	}

	void SceneRenderer::FlushDrawList()
	{
		m_command_buffer->Begin();
		if (m_resources_created && m_viewport_width > 0 && m_viewport_height > 0)
		{
			PreRender();

			GeometryPass();

			CompositePass();
		}
		else
		{
			ClearPass();
		}

		m_command_buffer->End();
		m_command_buffer->Submit();

		m_scene_data = {};
	}

	void SceneRenderer::PreRender()
	{
		// #TODO
	}

	void SceneRenderer::ClearPass()
	{
		RenderCommand::BeginRenderPass(m_command_buffer, m_composite_pipeline->GetSpecification().render_pass, true);
		RenderCommand::EndRenderPass(m_command_buffer);
	}

	void SceneRenderer::ClearPass(IntrusiveRef<RenderPass> render_pass, bool explicit_clear /*= false*/)
	{
		KB_CORE_INFO("Clear pass being called for renderpass '{0}'", render_pass->GetSpecification().debug_name);
		RenderCommand::BeginRenderPass(m_command_buffer, render_pass, explicit_clear);
		RenderCommand::EndRenderPass(m_command_buffer);
	}

	void SceneRenderer::GeometryPass()
	{
		m_gpu_time_query_indices.geometry_pass_query = m_command_buffer->BeginTimestampQuery();
		RenderCommand::BeginRenderPass(m_command_buffer, m_geometry_pipeline->GetSpecification().render_pass);

		// #TODO

		RenderCommand::EndRenderPass(m_command_buffer);
		m_command_buffer->EndTimestampQuery(m_gpu_time_query_indices.geometry_pass_query);
	}

	void SceneRenderer::CompositePass()
	{
		m_gpu_time_query_indices.composite_pass_query = m_command_buffer->BeginTimestampQuery();
		RenderCommand::BeginRenderPass(m_command_buffer, m_composite_pipeline->GetSpecification().render_pass, true);

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

		RenderCommand::SubmitFullscreenQuad(m_command_buffer, m_composite_pipeline, nullptr, m_composite_material);

		RenderCommand::EndRenderPass(m_command_buffer);
		m_command_buffer->EndTimestampQuery(m_gpu_time_query_indices.composite_pass_query);
	}

}
