#include "kablunkpch.h"

#include "Kablunk/Renderer/SceneRenderer.h"
#include "Kablunk/Renderer/Renderer.h"
#include "Kablunk/Renderer/Renderer2D.h"

#include "Platform/Vulkan/VulkanRendererAPI.h"

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

		uint32_t frames_in_flight = Renderer::GetConfig().frames_in_flight;

		FramebufferSpecification framebuffer_spec;
		framebuffer_spec.Attachments = { ImageFormat::RGBA32F, ImageFormat::RGBA16F, ImageFormat::RGBA16F, ImageFormat::Depth };
		framebuffer_spec.samples = 1;
		framebuffer_spec.clear_color = { 0.0f, 0.0f, 0.0f, 1.0f };
		framebuffer_spec.debug_name = "Renderer2D";

		RenderPassSpecification render_pass_spec{};
		render_pass_spec.target_framebuffer = Framebuffer::Create(framebuffer_spec);
		render_pass_spec.debug_name = "Renderer2D";

		// Quad
		{
			PipelineSpecification pipeline_spec;
			pipeline_spec.debug_name = "QuadPipeline";
			pipeline_spec.shader = Renderer::GetShaderLibrary()->Get("Renderer2D_Quad");
			pipeline_spec.backface_culling = false;
			pipeline_spec.layout = {
				{ ShaderDataType::Float3, "a_Position" },
				{ ShaderDataType::Float4, "a_Color" },
				{ ShaderDataType::Float2, "a_TexCoord" },
				{ ShaderDataType::Float, "a_TexIndex" },
				{ ShaderDataType::Float, "a_TilingFactor" },
				{ ShaderDataType::Int, "a_EntityID" }
			};
			pipeline_spec.render_pass = RenderPass::Create(render_pass_spec);

			m_quad_pipeline = Pipeline::Create(pipeline_spec);
		}

		// Circle
		{
			PipelineSpecification pipeline_spec;
			pipeline_spec.debug_name = "CirclePipeline";
			pipeline_spec.shader = Renderer::GetShaderLibrary()->Get("Renderer2D_Circle");
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
			pipeline_spec.render_pass = RenderPass::Create(render_pass_spec);

			m_circle_pipeline = Pipeline::Create(pipeline_spec);
		}

		// Geometry
		{
			FramebufferSpecification geometry_framebuffer_spec;
			geometry_framebuffer_spec.Attachments = { ImageFormat::RGBA32F, ImageFormat::RGBA16F, ImageFormat::RGBA16F, ImageFormat::Depth };
			geometry_framebuffer_spec.samples = 1;
			geometry_framebuffer_spec.clear_color = { 0.0f, 0.0f, 0.0f, 1.0f };
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
			FramebufferSpecification compFramebufferSpec;
			compFramebufferSpec.debug_name = "SceneComposite";
			compFramebufferSpec.clear_color = { 0.5f, 0.1f, 0.1f, 1.0f };
			compFramebufferSpec.swap_chain_target = m_specification.swap_chain_target;

			// No depth for swapchain
			if (m_specification.swap_chain_target)
				compFramebufferSpec.Attachments = { ImageFormat::RGBA };
			else
				compFramebufferSpec.Attachments = { ImageFormat::RGBA, ImageFormat::Depth };

			IntrusiveRef<Framebuffer> framebuffer = Framebuffer::Create(compFramebufferSpec);

			RenderPassSpecification renderPassSpec;
			renderPassSpec.target_framebuffer = framebuffer;
			renderPassSpec.debug_name = "SceneComposite";

			PipelineSpecification pipelineSpecification;
			pipelineSpecification.layout = {
				{ ShaderDataType::Float3, "a_Position" },
				{ ShaderDataType::Float2, "a_TexCoord" }
			};
			pipelineSpecification.backface_culling = false;
			pipelineSpecification.shader = Renderer::GetShaderLibrary()->Get("scene_composite");
			pipelineSpecification.render_pass = RenderPass::Create(renderPassSpec);
			pipelineSpecification.debug_name = "SceneComposite";
			pipelineSpecification.depth_write = false;
			m_composite_pipeline = Pipeline::Create(pipelineSpecification);

		}

		IntrusiveRef<SceneRenderer> instance = this;
		RenderCommand::Submit([instance]() mutable
			{
				instance->m_resources_created = true;
			});
	}

	void SceneRenderer::SetScene(IntrusiveRef<Scene> context)
	{
		KB_CORE_ASSERT(context, "Scene context is nullptr!");
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
			m_quad_pipeline->GetSpecification().render_pass->GetSpecification().target_framebuffer->Resize(m_viewport_width, m_viewport_height);
			m_circle_pipeline->GetSpecification().render_pass->GetSpecification().target_framebuffer->Resize(m_viewport_width, m_viewport_height);
			m_geometry_pipeline->GetSpecification().render_pass->GetSpecification().target_framebuffer->Resize(m_viewport_width, m_viewport_height);
			m_composite_pipeline->GetSpecification().render_pass->GetSpecification().target_framebuffer->Resize(m_viewport_width, m_viewport_height);

			m_needs_resize = false;
		}

		auto& scene_camera = m_scene_data.camera;
		const auto view_projection = scene_camera.camera.GetProjection() * scene_camera.view_mat;
		const glm::vec3 camera_position = glm::inverse(scene_camera.view_mat)[3];

		const auto inverse_view_projection = glm::inverse(view_projection);
		
		// #TODO set camera uniform buffer

		
		Renderer2D::BeginScene(camera.camera, camera.view_mat);
	}

	void SceneRenderer::EndScene()
	{
		IntrusiveRef<SceneRenderer> instance = this;
		s_thread_pool.emplace_back(([instance]() mutable
			{
				instance->FlushDrawList();
			}
		));

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

		return m_composite_pipeline->GetSpecification().render_pass->GetSpecification().target_framebuffer->GetImage();
	}

	void SceneRenderer::OnImGuiRender()
	{
		// #TODO
	}

	void SceneRenderer::WaitForThreads()
	{
		for (auto& thread : s_thread_pool)
			thread.join();

		s_thread_pool.clear();
	}

	void SceneRenderer::FlushDrawList()
	{
		
		if (m_resources_created && m_viewport_width > 0 && m_viewport_height > 0)
		{
			PreRender();

			m_command_buffer->Begin();

			GeometryPass();
			TwoDimensionalPass();

			CompositePass();
		}
		else
		{
			m_command_buffer->Begin();
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
		VulkanRendererAPI* vulkan_renderer = dynamic_cast<VulkanRendererAPI*>(RenderCommand::GetRenderer());
		vulkan_renderer->BeginRenderPass(m_command_buffer, m_composite_pipeline->GetSpecification().render_pass, true);
		vulkan_renderer->EndRenderPass(m_command_buffer);
	}

	void SceneRenderer::ClearPass(IntrusiveRef<RenderPass> render_pass, bool explicit_clear /*= false*/)
	{
		VulkanRendererAPI* vulkan_renderer = dynamic_cast<VulkanRendererAPI*>(RenderCommand::GetRenderer());

		vulkan_renderer->BeginRenderPass(m_command_buffer, render_pass, explicit_clear);
		vulkan_renderer->EndRenderPass(m_command_buffer);
	}

	void SceneRenderer::GeometryPass()
	{
		// #TODO
	}

	void SceneRenderer::TwoDimensionalPass()
	{
		IntrusiveRef<SceneRenderer> instance = this;

		// Quad
		m_command_buffer->BeginTimestampQuery();
		Renderer2D::EndScene();
		m_command_buffer->EndTimestampQuery(m_gpu_time_query_indices.two_dimensional_pass_query);
	}

	void SceneRenderer::CompositePass()
	{
		m_command_buffer->BeginTimestampQuery();
		VulkanRendererAPI* vulkan_renderer = dynamic_cast<VulkanRendererAPI*>(RenderCommand::GetRenderer());
		vulkan_renderer->BeginRenderPass(m_command_buffer, m_composite_pipeline->GetSpecification().render_pass);

		auto framebuffer = m_geometry_pipeline->GetSpecification().render_pass->GetSpecification().target_framebuffer;
		float exposure = 1.0f; // #TODO dynamic based off camera
		int texture_samples = framebuffer->GetSpecification().samples;

		RenderCommand::SubmitFullscreenQuad(m_command_buffer, m_composite_pipeline, nullptr, nullptr, nullptr);

		vulkan_renderer->EndRenderPass(m_command_buffer);
		m_command_buffer->EndTimestampQuery(m_gpu_time_query_indices.composite_pass_query);
	}

}
