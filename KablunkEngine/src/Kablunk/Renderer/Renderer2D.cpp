#include "kablunkpch.h"
#include "Kablunk/Renderer/Renderer2D.h"

#include "Kablunk/Renderer/VertexArray.h"
#include "Kablunk/Renderer/Shader.h"
#include "Kablunk/Renderer/RenderCommand.h"

#include <glm/gtc/matrix_transform.hpp>

namespace Kablunk
{
	struct QuadVertex
	{
		glm::vec3 Position;
		glm::vec4 Color;
		glm::vec2 TexCoord;
		float TexIndex;
		float TilingFactor;
		// TODO color, texid
	};


	struct Renderer2DData
	{
		static const uint32_t Max_quads			= 40'000;
		static const uint32_t Max_vertices		= Max_quads * 4;
		static const uint32_t Max_indices		= Max_quads * 6;
		static const uint32_t Max_texture_slots	= 32;
		glm::vec4 Quad_vertex_positions[4]		= {};

		Ref <VertexArray> Quad_vertex_array;
		Ref <VertexBuffer> Quad_vertex_buffer;
		Ref <Shader> Texture_shader;
		Ref <Texture2D> White_texture;
		
		QuadVertex* Quad_vertex_buffer_base_ptr	= nullptr;
		QuadVertex* Quad_vertex_buffer_ptr		= nullptr;

		uint32_t Quad_count						= 0;
		uint32_t Quad_index_count				= 0;
		

		
		uint32_t Texture_slot_index				= 1; //0 = white texture
		
		// TODO: change to asset handle when implemented
		std::array<Ref<Texture2D>, Max_texture_slots> Texture_slots;

		Renderer2D::Renderer2DStats Stats;
	};

	static Renderer2DData s_renderer_data;

	void Renderer2D::Init()
	{
		KB_PROFILE_FUNCTION();

		s_renderer_data.Quad_vertex_array = VertexArray::Create();

		s_renderer_data.Quad_vertex_buffer = VertexBuffer::Create(s_renderer_data.Max_vertices * sizeof(QuadVertex));

		s_renderer_data.Quad_vertex_buffer->SetLayout({
			{ ShaderDataType::Float3, "a_Position" },
			{ ShaderDataType::Float4, "a_Color"},
			{ ShaderDataType::Float2, "a_TexCoord" },
			{ ShaderDataType::Float, "a_TexIndex" },
			{ ShaderDataType::Float, "a_TilingFactor" }
		});
		s_renderer_data.Quad_vertex_array->AddVertexBuffer(s_renderer_data.Quad_vertex_buffer);

		s_renderer_data.Quad_vertex_buffer_base_ptr = new QuadVertex[s_renderer_data.Max_vertices];

		uint32_t* quad_indices = new uint32_t[s_renderer_data.Max_indices];

		uint32_t offset = 0;
		for (uint32_t i = 0; i < s_renderer_data.Max_indices; i += 6)
		{
			quad_indices[i + uint32_t{ 0 }] = offset + uint32_t{0};
			quad_indices[i + uint32_t{ 1 }] = offset + uint32_t{1};
			quad_indices[i + uint32_t{ 2 }] = offset + uint32_t{2};
							  		 	  					  
			quad_indices[i + uint32_t{ 3 }] = offset + uint32_t{2};
			quad_indices[i + uint32_t{ 4 }] = offset + uint32_t{3};
			quad_indices[i + uint32_t{ 5 }] = offset + uint32_t{0};

			offset += uint32_t{ 4 };
		}

		Ref<IndexBuffer> quad_index_buffer = IndexBuffer::Create(quad_indices, s_renderer_data.Max_indices);
		s_renderer_data.Quad_vertex_array->SetIndexBuffer(quad_index_buffer);
		delete[] quad_indices;

		s_renderer_data.White_texture = Texture2D::Create(1, 1);

		int32_t samplers[s_renderer_data.Max_texture_slots];
		for (int32_t i = 0; i < s_renderer_data.Max_texture_slots; ++i)
			samplers[i] = i;

		s_renderer_data.Texture_shader = Shader::Create("assets/shaders/Texture.glsl");
		s_renderer_data.Texture_shader->Bind();
		s_renderer_data.Texture_shader->SetIntArray("u_Textures", samplers, s_renderer_data.Max_texture_slots);

		// Set all the texture slots to zero
		//memset(s_RendererData.TextureSlots.data(), 0, s_RendererData.TextureSlots.size() * sizeof(uint32_t));
		s_renderer_data.Texture_slots[0] = s_renderer_data.White_texture;

		s_renderer_data.Quad_vertex_positions[0] = { -0.5f, -0.5f, 0.0f, 1.0f };
		s_renderer_data.Quad_vertex_positions[1] = {  0.5f, -0.5f, 0.0f, 1.0f };
		s_renderer_data.Quad_vertex_positions[2] = {  0.5f,  0.5f, 0.0f, 1.0f };
		s_renderer_data.Quad_vertex_positions[3] = { -0.5f,  0.5f, 0.0f, 1.0f };
	}

	void Renderer2D::Shutdown()
	{
		KB_PROFILE_FUNCTION();

		// Cleanup memory
		delete[] s_renderer_data.Quad_vertex_buffer_base_ptr;
	}

	void Renderer2D::BeginScene(const Camera& camera, const glm::mat4& transform)
	{
		BeginScene(RenderCamera{ camera.GetProjection(), transform });
	}


	void Renderer2D::BeginScene(const RenderCamera& render_camera)
	{
		glm::mat4 view_projection = render_camera.Projection * glm::inverse(render_camera.Transform);

		s_renderer_data.Texture_shader->Bind();
		s_renderer_data.Texture_shader->SetMat4("u_ViewProjection", view_projection);

		StartNewBatch();
	}

	void Renderer2D::BeginScene(const OrthographicCamera& camera)
	{
		KB_PROFILE_FUNCTION();

		s_renderer_data.Texture_shader->Bind();
		s_renderer_data.Texture_shader->SetMat4("u_ViewProjection", camera.GetViewProjectionMatrix());

		StartNewBatch();
	}

	void Renderer2D::EndScene()
	{
		KB_PROFILE_FUNCTION();

		Flush();
	}

	void Renderer2D::Flush()
	{
		if (s_renderer_data.Quad_index_count == 0)
			return;

		KB_PROFILE_FUNCTION();

		uint32_t data_size = (uint32_t)((uint8_t*)s_renderer_data.Quad_vertex_buffer_ptr - (uint8_t*)s_renderer_data.Quad_vertex_buffer_base_ptr);
		s_renderer_data.Quad_vertex_buffer->SetData(s_renderer_data.Quad_vertex_buffer_base_ptr, data_size);
		
		for (uint32_t i = 0; i < s_renderer_data.Texture_slot_index; ++i)
		{
			s_renderer_data.Texture_slots[i]->Bind(i);
		}

		RenderCommand::DrawIndexed(s_renderer_data.Quad_vertex_array, s_renderer_data.Quad_index_count);
	}

	// ========================
	//   Draw Quad with Color
	// ========================

	void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color)
	{
		DrawQuad({ position.x, position.y, 0.0f }, size, color);
	}

	void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color)
	{
		glm::mat4 transform = glm::translate(glm::mat4(1.0f), position)
			* glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });

		DrawQuad(transform, color);
	}

	void Renderer2D::DrawQuad(const glm::mat4& transform, const glm::vec4& color)
	{
		if (s_renderer_data.Quad_count >= s_renderer_data.Max_quads)
			EndBatch();

		const float k_texture_index = 0.0f;
		const float k_tiling_factor = 1.0f;

		constexpr glm::vec2 textureCoords[] = { {0.0f, 0.0f}, { 1.0f, 0.0f}, { 1.0f, 1.0f}, { 0.0f, 1.0f } };
		constexpr size_t quadVertexCount = 4;

		for (uint32_t i = 0; i < quadVertexCount; ++i)
		{
			s_renderer_data.Quad_vertex_buffer_ptr->Position = transform * s_renderer_data.Quad_vertex_positions[i];
			s_renderer_data.Quad_vertex_buffer_ptr->Color = color;
			s_renderer_data.Quad_vertex_buffer_ptr->TexCoord = textureCoords[i];
			s_renderer_data.Quad_vertex_buffer_ptr->TexIndex = k_texture_index;
			s_renderer_data.Quad_vertex_buffer_ptr->TilingFactor = k_tiling_factor;
			s_renderer_data.Quad_vertex_buffer_ptr++;
		}

		s_renderer_data.Quad_index_count += 6;
		s_renderer_data.Quad_count++;

		s_renderer_data.Stats.Quad_count += 1;
	}

	// ==========================
	//   Draw Quad with Texture
	// ==========================

	void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size, const Ref<Texture2D>& texture, float tiling_factor, const glm::vec4& tint_color)
	{
		DrawQuad({ position.x, position.y, 0.0f }, size, texture, tiling_factor, tint_color);
	}

	void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size, const Ref<Texture2D>& texture, float tiling_factor, const glm::vec4& tint_color)
	{
		glm::mat4 transform = glm::translate(glm::mat4(1.0f), position)
			* glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });
		
		DrawQuad(transform, texture, tiling_factor, tint_color);
	}

	void Renderer2D::DrawQuad(const glm::mat4& transform, const Ref<Texture2D>& texture, float tiling_factor, const glm::vec4& tint_color)
	{
		if (s_renderer_data.Quad_count + 1 > s_renderer_data.Max_quads)
			EndBatch();


		//constexpr glm::vec4 color{ 1.0f, 1.0f, 1.0f, 1.0f };
		float texture_index = 0.0f;
		for (uint32_t i = 1; i < s_renderer_data.Texture_slot_index; ++i)
		{
			// Dereference shared_ptrs and compare the textures
			if (*s_renderer_data.Texture_slots[i].get() == *texture.get())
				texture_index = (float)i;
			
		}

		if (texture_index == 0.0f)
		{
			texture_index = (float)s_renderer_data.Texture_slot_index;
			s_renderer_data.Texture_slots[s_renderer_data.Texture_slot_index++] = texture;
		}

		constexpr glm::vec2 texture_coords[] = { {0.0f, 0.0f}, { 1.0f, 0.0f}, { 1.0f, 1.0f}, { 0.0f, 1.0f } };
		constexpr size_t quad_vertex_count = 4;

		for (uint32_t i = 0; i < quad_vertex_count; ++i)
		{
			s_renderer_data.Quad_vertex_buffer_ptr->Position = transform * s_renderer_data.Quad_vertex_positions[i];
			s_renderer_data.Quad_vertex_buffer_ptr->Color = tint_color;
			s_renderer_data.Quad_vertex_buffer_ptr->TexCoord = texture_coords[i];
			s_renderer_data.Quad_vertex_buffer_ptr->TexIndex = texture_index;
			s_renderer_data.Quad_vertex_buffer_ptr->TilingFactor = tiling_factor;
			s_renderer_data.Quad_vertex_buffer_ptr++;
		}
		s_renderer_data.Quad_index_count += 6;
		s_renderer_data.Quad_count++;

		s_renderer_data.Stats.Quad_count += 1;
	}

	// ================================
	//   Draw Rotated Quad with Color
	// ================================

	void Renderer2D::DrawRotatedQuad(const glm::vec2& position, const glm::vec2& size, float rotation, const glm::vec4& color)
	{
		DrawRotatedQuad({ position.x, position.y, 1.0f }, size, rotation, color);
	}

	void Renderer2D::DrawRotatedQuad(const glm::vec3& position, const glm::vec2& size, float rotation, const glm::vec4& color)
	{
		glm::mat4 transform = glm::translate(glm::mat4(1.0f), position)
			* glm::rotate(glm::mat4(1.0f), glm::radians(rotation), { 0.0f, 0.0f, 1.0f })
			* glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });

		DrawRotatedQuad(transform, color);
	}

	void Renderer2D::DrawRotatedQuad(const glm::mat4& transform, const glm::vec4& color)
	{
		if (s_renderer_data.Quad_count + 1 > s_renderer_data.Max_quads)
			EndBatch();

		constexpr float k_texture_index = 0.0f;
		constexpr float k_tiling_factor = 1.0f;

		constexpr glm::vec2 texture_coords[] = { {0.0f, 0.0f}, { 1.0f, 0.0f}, { 1.0f, 1.0f}, { 0.0f, 1.0f } };
		constexpr size_t quad_vertex_count = 4;

		for (uint32_t i = 0; i < quad_vertex_count; ++i)
		{
			s_renderer_data.Quad_vertex_buffer_ptr->Position = transform * s_renderer_data.Quad_vertex_positions[i];
			s_renderer_data.Quad_vertex_buffer_ptr->Color = color;
			s_renderer_data.Quad_vertex_buffer_ptr->TexCoord = texture_coords[i];
			s_renderer_data.Quad_vertex_buffer_ptr->TexIndex = k_texture_index;
			s_renderer_data.Quad_vertex_buffer_ptr->TilingFactor = k_tiling_factor;
			s_renderer_data.Quad_vertex_buffer_ptr++;

		}

		s_renderer_data.Quad_index_count += 6;
		s_renderer_data.Quad_count++;

		s_renderer_data.Stats.Quad_count += 1;
	}

	void Renderer2D::DrawRotatedQuad(const glm::vec2& position, const glm::vec2& size, float rotation, const Ref<Texture2D>& texture, float tiling_factor, const glm::vec4& tint_color)
	{
		DrawRotatedQuad({ position.x, position.y, 1.0f }, size, rotation, texture, tiling_factor, tint_color);
	}

	void Renderer2D::DrawRotatedQuad(const glm::vec3& position, const glm::vec2& size, float rotation, const Ref<Texture2D>& texture, float tiling_factor, const glm::vec4& tint_color)
	{
		glm::mat4 transform = glm::translate(glm::mat4(1.0f), position)
			* glm::rotate(glm::mat4(1.0f), glm::radians(rotation), {0.0f, 0.0f, 1.0f})
			* glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });

		DrawRotatedQuad(transform, texture, tiling_factor, tint_color);
	}

	void Renderer2D::DrawRotatedQuad(const glm::mat4& transform, const Ref<Texture2D>& texture, float tiling_factor, const glm::vec4& tint_color)
	{
		if (s_renderer_data.Quad_count + 1 > s_renderer_data.Max_quads)
			EndBatch();

		float texture_index = 0.0f;
		for (uint32_t i = 1; i < s_renderer_data.Texture_slot_index; ++i)
		{
			// Dereference shared_ptrs and compare the textures
			if (*s_renderer_data.Texture_slots[i].get() == *texture.get())
			{
				texture_index = (float)i;
			}
		}

		if (texture_index == 0.0f)
		{
			texture_index = (float)s_renderer_data.Texture_slot_index;
			s_renderer_data.Texture_slots[s_renderer_data.Texture_slot_index++] = texture;
		}

		constexpr glm::vec2 texture_coords[] = { {0.0f, 0.0f}, { 1.0f, 0.0f}, { 1.0f, 1.0f}, { 0.0f, 1.0f } };
		constexpr size_t quad_vertex_count = 4;
		for (uint32_t i = 0; i < quad_vertex_count; ++i)
		{
			s_renderer_data.Quad_vertex_buffer_ptr->Position = transform * s_renderer_data.Quad_vertex_positions[i];
			s_renderer_data.Quad_vertex_buffer_ptr->Color = tint_color;
			s_renderer_data.Quad_vertex_buffer_ptr->TexCoord = texture_coords[i];
			s_renderer_data.Quad_vertex_buffer_ptr->TexIndex = texture_index;
			s_renderer_data.Quad_vertex_buffer_ptr->TilingFactor = tiling_factor;
			s_renderer_data.Quad_vertex_buffer_ptr++;

		}

		s_renderer_data.Quad_index_count += 6;
		s_renderer_data.Quad_count++;

		s_renderer_data.Stats.Quad_count += 1;
	}

	void Renderer2D::ResetStats()
	{
		s_renderer_data.Stats.Draw_calls = 0;
		s_renderer_data.Stats.Quad_count = 0;
	}

	Renderer2D::Renderer2DStats Renderer2D::GetStats() { return s_renderer_data.Stats; }

	void Renderer2D::StartNewBatch()
	{
		s_renderer_data.Quad_count = 0;
		s_renderer_data.Quad_index_count = 0;
		s_renderer_data.Quad_vertex_buffer_ptr = s_renderer_data.Quad_vertex_buffer_base_ptr;

		s_renderer_data.Texture_slot_index = 1;

		s_renderer_data.Stats.Draw_calls++;
	}

	void Renderer2D::EndBatch()
	{
		Flush();
		StartNewBatch();
	}

}