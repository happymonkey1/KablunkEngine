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


namespace Kablunk
{
	struct GenericRenderer2DData
	{
		static const uint32_t max_quads = 20'000;
		static const uint32_t max_vertices = max_quads * 4;
		static const uint32_t max_indices = max_quads * 6;
		static const uint32_t max_texture_slots = 32;
		glm::vec4 quad_vertex_positions[4] = {};

		IntrusiveRef <VertexBuffer> quad_vertex_buffer;
		IntrusiveRef <IndexBuffer> quad_index_buffer;

		IntrusiveRef <VertexBuffer> circle_vertex_buffer;
		IntrusiveRef <IndexBuffer> circle_index_buffer;

		IntrusiveRef<Shader> quad_shader;
		IntrusiveRef<Shader> circle_shader;

		IntrusiveRef <Texture2D> white_texture;

		QuadVertex* quad_vertex_buffer_base_ptr = nullptr;
		QuadVertex* quad_vertex_buffer_ptr = nullptr;
		uint32_t quad_count = 0;
		uint32_t quad_index_count = 0;

		CircleVertex* circle_vertex_buffer_base_ptr = nullptr;
		CircleVertex* circle_vertex_buffer_ptr = nullptr;
		uint32_t circle_count = 0;
		uint32_t circle_index_count = 0;

		uint32_t texture_slot_index = 1; //0 = white texture

		// TODO: change to asset handle when implemented
		std::array<IntrusiveRef<Texture2D>, max_texture_slots> texture_slots;

		IntrusiveRef<RenderCommandBuffer> render_command_buffer;

		IntrusiveRef<Pipeline> quad_pipeline;
		IntrusiveRef<Pipeline> circle_pipeline;

		IntrusiveRef<Material> quad_material;
		IntrusiveRef<Material> circle_material;

		IntrusiveRef<UniformBufferSet> uniform_buffer_set;

		Camera camera;
		glm::mat4 camera_transform;
		glm::mat4 camera_view_projection;

		Renderer2DSpecification specification;

		Renderer2D::Renderer2DStats Stats = {};
	};

	static GenericRenderer2DData s_renderer_data;

	void Renderer2D::Init(Renderer2DSpecification spec)
	{
		KB_PROFILE_FUNCTION();

		s_renderer_data = {};
		s_renderer_data.specification = spec;

		if (s_renderer_data.specification.swap_chain_target)
			s_renderer_data.render_command_buffer = RenderCommandBuffer::CreateFromSwapChain("Renderer2D");
		else
			s_renderer_data.render_command_buffer = RenderCommandBuffer::Create(0, "Renderer2D");

		s_renderer_data.quad_vertex_buffer = VertexBuffer::Create(s_renderer_data.max_vertices * sizeof(QuadVertex));
		s_renderer_data.quad_vertex_buffer_base_ptr = new QuadVertex[s_renderer_data.max_vertices];

		uint32_t* quad_indices = new uint32_t[s_renderer_data.max_indices];

		uint32_t offset = 0;
		for (uint32_t i = 0; i < s_renderer_data.max_indices; i += 6)
		{
			quad_indices[i + uint32_t{ 0 }] = offset + uint32_t{ 0 };
			quad_indices[i + uint32_t{ 1 }] = offset + uint32_t{ 1 };
			quad_indices[i + uint32_t{ 2 }] = offset + uint32_t{ 2 };

			quad_indices[i + uint32_t{ 3 }] = offset + uint32_t{ 2 };
			quad_indices[i + uint32_t{ 4 }] = offset + uint32_t{ 3 };
			quad_indices[i + uint32_t{ 5 }] = offset + uint32_t{ 0 };

			offset += uint32_t{ 4 };
		}

		s_renderer_data.quad_index_buffer = IndexBuffer::Create(quad_indices, s_renderer_data.max_indices);
		s_renderer_data.circle_index_buffer = IndexBuffer::Create(quad_indices, s_renderer_data.max_indices);
		delete[] quad_indices;

		s_renderer_data.circle_vertex_buffer = VertexBuffer::Create(s_renderer_data.max_vertices * sizeof(CircleVertex));
		s_renderer_data.circle_vertex_buffer_base_ptr = new CircleVertex[s_renderer_data.max_vertices];

		s_renderer_data.white_texture = Texture2D::Create(ImageFormat::RGBA32F, 1, 1);

		int32_t samplers[s_renderer_data.max_texture_slots];
		for (int32_t i = 0; i < s_renderer_data.max_texture_slots; ++i)
			samplers[i] = i;

		s_renderer_data.quad_shader = Renderer::GetShaderLibrary()->Get("Renderer2D_Quad");
		s_renderer_data.circle_shader = Renderer::GetShaderLibrary()->Get("Renderer2D_Circle");

		// Set all the texture slots to zero
		//memset(s_RendererData.TextureSlots.data(), 0, s_RendererData.TextureSlots.size() * sizeof(uint32_t));
		s_renderer_data.texture_slots[0] = s_renderer_data.white_texture;

		s_renderer_data.quad_vertex_positions[0] = { -0.5f, -0.5f, 0.0f, 1.0f };
		s_renderer_data.quad_vertex_positions[1] = { 0.5f, -0.5f, 0.0f, 1.0f };
		s_renderer_data.quad_vertex_positions[2] = { 0.5f,  0.5f, 0.0f, 1.0f };
		s_renderer_data.quad_vertex_positions[3] = { -0.5f,  0.5f, 0.0f, 1.0f };

		// Create framebuffer
		FramebufferSpecification framebuffer_spec{};
		framebuffer_spec.Attachments = { ImageFormat::RGBA32F, ImageFormat::Depth };
		framebuffer_spec.samples = 1;
		framebuffer_spec.clear_on_load = false;
		framebuffer_spec.clear_color = { 0.0f, 0.0f, 0.0f, 1.0f };
		framebuffer_spec.debug_name = "Renderer2D Framebuffer";

		IntrusiveRef<Framebuffer> framebuffer = Framebuffer::Create(framebuffer_spec);

		RenderPassSpecification render_pass_spec{};
		render_pass_spec.target_framebuffer = framebuffer;
		render_pass_spec.debug_name = "Renderer2D";

		IntrusiveRef<RenderPass> render_pass = RenderPass::Create(render_pass_spec);

		// Create quad pipeline
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
			pipeline_spec.render_pass = render_pass;

			s_renderer_data.quad_pipeline = Pipeline::Create(pipeline_spec);
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
			pipeline_spec.render_pass = render_pass;

			s_renderer_data.circle_pipeline = Pipeline::Create(pipeline_spec);
		}

		s_renderer_data.quad_material = Material::Create(s_renderer_data.quad_shader);
		s_renderer_data.circle_material = Material::Create(s_renderer_data.circle_shader);

		uint32_t frames_in_flight = Renderer::GetConfig().frames_in_flight;
		s_renderer_data.uniform_buffer_set = UniformBufferSet::Create(frames_in_flight);
		s_renderer_data.uniform_buffer_set->Create(sizeof(CameraDataUB), 0);
	}

	void Renderer2D::Shutdown()
	{
		KB_PROFILE_FUNCTION();

		// Cleanup memory
		delete[] s_renderer_data.quad_vertex_buffer_base_ptr;
		delete[] s_renderer_data.circle_vertex_buffer_base_ptr;
	}

	IntrusiveRef<Texture2D> Renderer2D::GetWhiteTexture()
	{
		return s_renderer_data.white_texture;
	}

	void Renderer2D::BeginScene(const Camera& camera, const glm::mat4& transform)
	{
		s_renderer_data.camera = camera;
		s_renderer_data.camera_transform = transform;

		auto view_proj = camera.GetProjection() * glm::inverse(transform);
		s_renderer_data.camera_view_projection = view_proj;

		IntrusiveRef<UniformBufferSet> uniform_buffer_set = s_renderer_data.uniform_buffer_set;
		RenderCommand::Submit([uniform_buffer_set, view_proj]() mutable
			{
				uint32_t buffer_index = Renderer::GetCurrentFrameIndex();
				uniform_buffer_set->Get(0, 0, buffer_index)->RT_SetData(&view_proj, sizeof(CameraDataUB));
			});

		s_renderer_data.Stats = {};

		StartNewBatch();
	}

	void Renderer2D::BeginScene(const EditorCamera& camera)
	{
		Renderer2D::BeginScene(camera, camera.GetViewMatrix());

		StartNewBatch();
	}

	void Renderer2D::BeginScene(const OrthographicCamera& camera)
	{
		StartNewBatch();
	}

	

	void Renderer2D::EndScene()
	{
		Flush();
	}

	void Renderer2D::Flush()
	{
		KB_CORE_ASSERT(s_renderer_data.Stats.batch_count < 1, "Multiple batches per frame not supported!");
		
		s_renderer_data.render_command_buffer->Begin();
		RenderCommand::BeginRenderPass(s_renderer_data.render_command_buffer, s_renderer_data.quad_pipeline->GetSpecification().render_pass);
		
		// Quad
		{
			uint32_t data_size = (uint32_t)((uint8_t*)s_renderer_data.quad_vertex_buffer_ptr - (uint8_t*)s_renderer_data.quad_vertex_buffer_base_ptr);
			s_renderer_data.quad_vertex_buffer->SetData(s_renderer_data.quad_vertex_buffer_base_ptr, data_size);

			// Set Textures
			auto& textures = s_renderer_data.texture_slots;
			for (uint32_t i = 0; i < s_renderer_data.texture_slot_index; i++)
			{
				if (textures[i])
					s_renderer_data.quad_material->Set("u_Textures", textures[i], i);
				else
					s_renderer_data.quad_material->Set("u_Textures", s_renderer_data.white_texture, i);
			}

			RenderCommand::RenderGeometry(s_renderer_data.render_command_buffer, s_renderer_data.quad_pipeline, s_renderer_data.uniform_buffer_set, nullptr, s_renderer_data.quad_material, s_renderer_data.quad_vertex_buffer, s_renderer_data.quad_index_buffer, glm::mat4{ 1.0f }, s_renderer_data.quad_index_count);
			s_renderer_data.Stats.Draw_calls++;
		}

		// Circle
		{
			uint32_t data_size = (uint32_t)((uint8_t*)s_renderer_data.circle_vertex_buffer_ptr - (uint8_t*)s_renderer_data.circle_vertex_buffer_base_ptr);
			s_renderer_data.circle_vertex_buffer->SetData(s_renderer_data.circle_vertex_buffer_base_ptr, data_size);

			RenderCommand::RenderGeometry(s_renderer_data.render_command_buffer, s_renderer_data.circle_pipeline, s_renderer_data.uniform_buffer_set, nullptr, s_renderer_data.circle_material, s_renderer_data.circle_vertex_buffer, s_renderer_data.circle_index_buffer, glm::mat4{ 1.0f }, s_renderer_data.circle_index_count);
			s_renderer_data.Stats.Draw_calls++;
		}

		RenderCommand::EndRenderPass(s_renderer_data.render_command_buffer);
		s_renderer_data.render_command_buffer->End();
		s_renderer_data.render_command_buffer->Submit();

		s_renderer_data.Stats.batch_count++;
	}

	IntrusiveRef<RenderPass> Renderer2D::GetTargetRenderPass()
	{
		return s_renderer_data.quad_pipeline->GetSpecification().render_pass;
	}

	void Renderer2D::SetTargetRenderPass(IntrusiveRef<RenderPass> render_pass)
	{
		if (s_renderer_data.quad_pipeline->GetSpecification().render_pass != render_pass)
		{
			// Quad pipeline
			{
				PipelineSpecification pipeline_spec = s_renderer_data.quad_pipeline->GetSpecification();
				pipeline_spec.render_pass = render_pass;
				s_renderer_data.quad_pipeline = Pipeline::Create(pipeline_spec);
			}

			// Circle pipeline
			{
				PipelineSpecification pipeline_spec = s_renderer_data.circle_pipeline->GetSpecification();
				pipeline_spec.render_pass = render_pass;
				s_renderer_data.circle_pipeline = Pipeline::Create(pipeline_spec);
			}
		}
	}

	void Renderer2D::OnRecreateSwapchain()
	{
		if (s_renderer_data.specification.swap_chain_target)
			s_renderer_data.render_command_buffer = RenderCommandBuffer::CreateFromSwapChain("Renderer2D");
	}

	// =========================
	//   Draw Quad From Entity
	// =========================

	void Renderer2D::DrawSprite(Entity entity)
	{
		auto transform = entity.GetComponent<TransformComponent>().GetTransform();

		auto& sprite_renderer_comp = entity.GetComponent<SpriteRendererComponent>();
		
		const auto& texture = sprite_renderer_comp.Texture.Get();
		auto tint_color = sprite_renderer_comp.Color;
		auto tiling_factor = sprite_renderer_comp.Tiling_factor;

		DrawQuad(transform, texture, tiling_factor, tint_color, static_cast<int32_t>(entity.GetHandle()));
	}

	// ==========================
	//   Draw Quad with Texture
	// ==========================

	void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size, const IntrusiveRef<Texture2D>& texture, float tiling_factor, const glm::vec4& tint_color)
	{
		DrawQuad({ position.x, position.y, 0.0f }, size, texture, tiling_factor, tint_color);
	}

	void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size, const IntrusiveRef<Texture2D>& texture, float tiling_factor, const glm::vec4& tint_color)
	{
		glm::mat4 transform = glm::translate(glm::mat4(1.0f), position)
			* glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });
		
		DrawQuad(transform, texture, tiling_factor, tint_color);
	}

	void Renderer2D::DrawQuad(const glm::mat4& transform, const IntrusiveRef<Texture2D>& texture, float tiling_factor, const glm::vec4& tint_color, int32_t entity_id)
	{
		if (s_renderer_data.quad_count + 1 > s_renderer_data.max_quads)
			EndBatch();

		//constexpr glm::vec4 color{ 1.0f, 1.0f, 1.0f, 1.0f };
		float texture_index = 0.0f;
		for (uint32_t i = 1; i < s_renderer_data.texture_slot_index; ++i)
		{
			// Dereference shared_ptrs and compare the textures
			if (*s_renderer_data.texture_slots[i].get() == *texture.get())
				texture_index = (float)i;
		}

		if (texture_index == 0.0f)
		{
			texture_index = (float)s_renderer_data.texture_slot_index;
			s_renderer_data.texture_slots[s_renderer_data.texture_slot_index++] = texture;
		}

		constexpr glm::vec2 texture_coords[] = { {0.0f, 0.0f}, { 1.0f, 0.0f}, { 1.0f, 1.0f}, { 0.0f, 1.0f } };
		constexpr size_t quad_vertex_count = 4;

		for (uint32_t i = 0; i < quad_vertex_count; ++i)
		{
			s_renderer_data.quad_vertex_buffer_ptr->Position = transform * s_renderer_data.quad_vertex_positions[i];
			s_renderer_data.quad_vertex_buffer_ptr->Color = tint_color;
			s_renderer_data.quad_vertex_buffer_ptr->TexCoord = texture_coords[i];
			s_renderer_data.quad_vertex_buffer_ptr->TexIndex = texture_index;
			s_renderer_data.quad_vertex_buffer_ptr->TilingFactor = tiling_factor;
			s_renderer_data.quad_vertex_buffer_ptr->EntityID = entity_id;
			s_renderer_data.quad_vertex_buffer_ptr++;
		}
		s_renderer_data.quad_index_count += 6;
		s_renderer_data.quad_count++;

		s_renderer_data.Stats.Quad_count += 1;
	}

	void Renderer2D::DrawCircle(const glm::mat4& transform, const glm::vec4& color, float radius /*= 0.5f*/, float thickness /*= 1.0f*/, float fade /*= 0.005f*/, int32_t entity_id /*= -1*/)
	{
		// #TODO implement for circles
		// if (s_renderer_data.Quad_count + 1 > s_renderer_data.Max_quads)
		// 	EndBatch();

		constexpr size_t circle_vertex_count = 4;

		for (uint32_t i = 0; i < circle_vertex_count; ++i)
		{
			s_renderer_data.circle_vertex_buffer_ptr->WorldPosition = transform * s_renderer_data.quad_vertex_positions[i];
			s_renderer_data.circle_vertex_buffer_ptr->LocalPosition = s_renderer_data.quad_vertex_positions[i] * 2.0f;
			s_renderer_data.circle_vertex_buffer_ptr->Color = color;
			s_renderer_data.circle_vertex_buffer_ptr->Radius = radius;
			s_renderer_data.circle_vertex_buffer_ptr->Thickness = thickness;
			s_renderer_data.circle_vertex_buffer_ptr->Fade = fade;
			s_renderer_data.circle_vertex_buffer_ptr->EntityID = entity_id;
			s_renderer_data.circle_vertex_buffer_ptr++;
		}
		s_renderer_data.circle_index_count += 6;
		s_renderer_data.circle_count++;

		s_renderer_data.Stats.Circle_count += 1;
	}

	void Renderer2D::ResetStats()
	{
		s_renderer_data.Stats.Draw_calls = 0;
		s_renderer_data.Stats.Quad_count = 0;
		s_renderer_data.Stats.Circle_count = 0;
	}

	Renderer2D::Renderer2DStats Renderer2D::GetStats() { return s_renderer_data.Stats; }

	void Renderer2D::StartNewBatch()
	{
		s_renderer_data.quad_count = 0;
		s_renderer_data.quad_index_count = 0;
		s_renderer_data.quad_vertex_buffer_ptr = s_renderer_data.quad_vertex_buffer_base_ptr;

		s_renderer_data.circle_count = 0;
		s_renderer_data.circle_index_count = 0;
		s_renderer_data.circle_vertex_buffer_ptr = s_renderer_data.circle_vertex_buffer_base_ptr;

		s_renderer_data.texture_slot_index = 1;
	}

	void Renderer2D::EndBatch()
	{
		Flush();
		StartNewBatch();
	}

}
