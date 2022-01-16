#include "kablunkpch.h"
#include "Kablunk/Renderer/Renderer2D.h"

#include "Kablunk/Renderer/VertexArray.h"
#include "Kablunk/Renderer/Shader.h"
#include "Kablunk/Renderer/RenderCommand.h"
#include "Kablunk/Renderer/UniformBuffer.h"
#include "Kablunk/Renderer/Renderer.h"

#include "Kablunk/Scene/Components.h"

#include "Platform/OpenGL/OpenGLRenderer2D.h"
#include "Platform/Vulkan/VulkanRenderer2D.h"

#include <glm/gtc/matrix_transform.hpp>


namespace Kablunk
{
	void Renderer2D::Init()
	{
		KB_PROFILE_FUNCTION();

		if (s_renderer)
			s_renderer->Shutdown();

		switch (RendererAPI::GetAPI())
		{
		case RendererAPI::RenderAPI_t::OpenGL:	s_renderer = new OpenGLRenderer2D{}; break;
		case RendererAPI::RenderAPI_t::Vulkan:	s_renderer = new VulkanRenderer2D{}; break;
		default:								KB_CORE_ASSERT(false, "Unknown RendererAPI!");
		}

		s_generic_renderer_data.Quad_vertex_buffer = VertexBuffer::Create(s_generic_renderer_data.Max_vertices * sizeof(QuadVertex));

		s_generic_renderer_data.Quad_vertex_buffer_base_ptr = new QuadVertex[s_generic_renderer_data.Max_vertices];

		uint32_t* quad_indices = new uint32_t[s_generic_renderer_data.Max_indices];

		uint32_t offset = 0;
		for (uint32_t i = 0; i < s_generic_renderer_data.Max_indices; i += 6)
		{
			quad_indices[i + uint32_t{ 0 }] = offset + uint32_t{ 0 };
			quad_indices[i + uint32_t{ 1 }] = offset + uint32_t{ 1 };
			quad_indices[i + uint32_t{ 2 }] = offset + uint32_t{ 2 };

			quad_indices[i + uint32_t{ 3 }] = offset + uint32_t{ 2 };
			quad_indices[i + uint32_t{ 4 }] = offset + uint32_t{ 3 };
			quad_indices[i + uint32_t{ 5 }] = offset + uint32_t{ 0 };

			offset += uint32_t{ 4 };
		}

		s_generic_renderer_data.Quad_index_buffer = IndexBuffer::Create(quad_indices, s_generic_renderer_data.Max_indices);
		delete[] quad_indices;

		s_generic_renderer_data.Circle_vertex_buffer = VertexBuffer::Create(s_generic_renderer_data.Max_vertices * sizeof(CircleVertex));

		s_generic_renderer_data.Circle_vertex_buffer_base_ptr = new CircleVertex[s_generic_renderer_data.Max_vertices];

		s_generic_renderer_data.White_texture = Texture2D::Create(ImageFormat::RGBA, 1, 1);

		int32_t samplers[s_generic_renderer_data.Max_texture_slots];
		for (int32_t i = 0; i < s_generic_renderer_data.Max_texture_slots; ++i)
			samplers[i] = i;

		s_generic_renderer_data.Quad_shader = Renderer::GetShaderLibrary()->Get("Renderer2D_Quad");
		//s_generic_renderer_data.Quad_shader->SetIntArray("u_Textures", samplers, s_generic_renderer_data.Max_texture_slots);

		s_generic_renderer_data.Circle_shader = Renderer::GetShaderLibrary()->Get("Renderer2D_Circle");

		// Set all the texture slots to zero
		//memset(s_RendererData.TextureSlots.data(), 0, s_RendererData.TextureSlots.size() * sizeof(uint32_t));
		s_generic_renderer_data.Texture_slots[0] = s_generic_renderer_data.White_texture;

		s_generic_renderer_data.Quad_vertex_positions[0] = { -0.5f, -0.5f, 0.0f, 1.0f };
		s_generic_renderer_data.Quad_vertex_positions[1] = { 0.5f, -0.5f, 0.0f, 1.0f };
		s_generic_renderer_data.Quad_vertex_positions[2] = { 0.5f,  0.5f, 0.0f, 1.0f };
		s_generic_renderer_data.Quad_vertex_positions[3] = { -0.5f,  0.5f, 0.0f, 1.0f };

		// Initialize API specific code
		s_renderer->Renderer2D_Init();
	}

	void Renderer2D::Shutdown()
	{
		KB_PROFILE_FUNCTION();

		// Cleanup memory
		delete[] s_generic_renderer_data.Quad_vertex_buffer_base_ptr;

		// Render API specific shutdown
		s_renderer->Renderer2D_Shutdown();
	}

	IntrusiveRef<Texture2D> Renderer2D::GetWhiteTexture()
	{
		return s_generic_renderer_data.White_texture;
	}

	void Renderer2D::BeginScene(const Camera& camera, const glm::mat4& transform)
	{
		StartNewBatch();
	}

	void Renderer2D::BeginScene(const EditorCamera& camera)
	{
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
		s_renderer->Renderer2D_Flush();
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
		if (s_generic_renderer_data.Quad_count + 1 > s_generic_renderer_data.Max_quads)
			EndBatch();

		//constexpr glm::vec4 color{ 1.0f, 1.0f, 1.0f, 1.0f };
		float texture_index = 0.0f;
		for (uint32_t i = 1; i < s_generic_renderer_data.Texture_slot_index; ++i)
		{
			// Dereference shared_ptrs and compare the textures
			if (*s_generic_renderer_data.Texture_slots[i].get() == *texture.get())
				texture_index = (float)i;
		}

		if (texture_index == 0.0f)
		{
			texture_index = (float)s_generic_renderer_data.Texture_slot_index;
			s_generic_renderer_data.Texture_slots[s_generic_renderer_data.Texture_slot_index++] = texture;
		}

		constexpr glm::vec2 texture_coords[] = { {0.0f, 0.0f}, { 1.0f, 0.0f}, { 1.0f, 1.0f}, { 0.0f, 1.0f } };
		constexpr size_t quad_vertex_count = 4;

		for (uint32_t i = 0; i < quad_vertex_count; ++i)
		{
			s_generic_renderer_data.Quad_vertex_buffer_ptr->Position = transform * s_generic_renderer_data.Quad_vertex_positions[i];
			s_generic_renderer_data.Quad_vertex_buffer_ptr->Color = tint_color;
			s_generic_renderer_data.Quad_vertex_buffer_ptr->TexCoord = texture_coords[i];
			s_generic_renderer_data.Quad_vertex_buffer_ptr->TexIndex = texture_index;
			s_generic_renderer_data.Quad_vertex_buffer_ptr->TilingFactor = tiling_factor;
			s_generic_renderer_data.Quad_vertex_buffer_ptr->EntityID = entity_id;
			s_generic_renderer_data.Quad_vertex_buffer_ptr++;
		}
		s_generic_renderer_data.Quad_index_count += 6;
		s_generic_renderer_data.Quad_count++;

		s_generic_renderer_data.Stats.Quad_count += 1;
	}

	void Renderer2D::DrawCircle(const glm::mat4& transform, const glm::vec4& color, float radius /*= 0.5f*/, float thickness /*= 1.0f*/, float fade /*= 0.005f*/, int32_t entity_id /*= -1*/)
	{
		// #TODO implement for circles
		// if (s_renderer_data.Quad_count + 1 > s_renderer_data.Max_quads)
		// 	EndBatch();

		constexpr size_t circle_vertex_count = 4;

		for (uint32_t i = 0; i < circle_vertex_count; ++i)
		{
			s_generic_renderer_data.Circle_vertex_buffer_ptr->WorldPosition = transform * s_generic_renderer_data.Quad_vertex_positions[i];
			s_generic_renderer_data.Circle_vertex_buffer_ptr->LocalPosition = s_generic_renderer_data.Quad_vertex_positions[i] * 2.0f;
			s_generic_renderer_data.Circle_vertex_buffer_ptr->Color = color;
			s_generic_renderer_data.Circle_vertex_buffer_ptr->Radius = radius;
			s_generic_renderer_data.Circle_vertex_buffer_ptr->Thickness = thickness;
			s_generic_renderer_data.Circle_vertex_buffer_ptr->Fade = fade;
			s_generic_renderer_data.Circle_vertex_buffer_ptr->EntityID = entity_id;
			s_generic_renderer_data.Circle_vertex_buffer_ptr++;
		}
		s_generic_renderer_data.Circle_index_count += 6;
		s_generic_renderer_data.Circle_count++;

		s_generic_renderer_data.Stats.Circle_count += 1;
	}

	void Renderer2D::SetSceneRenderer(IntrusiveRef<SceneRenderer> scene_renderer)
	{
		s_renderer->Renderer2D_SetSceneRenderer(scene_renderer);
	}

	void Renderer2D::ResetStats()
	{
		s_generic_renderer_data.Stats.Draw_calls = 0;
		s_generic_renderer_data.Stats.Quad_count = 0;
		s_generic_renderer_data.Stats.Circle_count = 0;
	}

	Renderer2D::Renderer2DStats Renderer2D::GetStats() { return s_generic_renderer_data.Stats; }

	void Renderer2D::StartNewBatch()
	{
		s_generic_renderer_data.Quad_count = 0;
		s_generic_renderer_data.Quad_index_count = 0;
		s_generic_renderer_data.Quad_vertex_buffer_ptr = s_generic_renderer_data.Quad_vertex_buffer_base_ptr;

		s_generic_renderer_data.Circle_count = 0;
		s_generic_renderer_data.Circle_index_count = 0;
		s_generic_renderer_data.Circle_vertex_buffer_ptr = s_generic_renderer_data.Circle_vertex_buffer_base_ptr;

		s_generic_renderer_data.Texture_slot_index = 1;
	}

	void Renderer2D::EndBatch()
	{
		Flush();
		StartNewBatch();
	}

}
