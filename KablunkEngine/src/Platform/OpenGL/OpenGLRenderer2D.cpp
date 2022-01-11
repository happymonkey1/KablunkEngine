#include "kablunkpch.h"

#include "Platform/OpenGL/OpenGLRenderer2D.h"
#include "Kablunk/Renderer/Shader.h"
#include "Kablunk/Renderer/VertexArray.h"

#include "Kablunk/Renderer/RenderCommand.h"

namespace Kablunk
{
	struct OpenGLRenderer2DData
	{
		IntrusiveRef <VertexArray> Circle_vertex_array;
		IntrusiveRef <VertexArray> Quad_vertex_array;
	};

	static OpenGLRenderer2DData s_renderer_data;

	void OpenGLRenderer2D::Renderer2D_Init()
	{
		//Quads
		s_generic_renderer_data.Quad_vertex_buffer->SetLayout({
			{ ShaderDataType::Float3, "a_Position" },
			{ ShaderDataType::Float4, "a_Color"},
			{ ShaderDataType::Float2, "a_TexCoord" },
			{ ShaderDataType::Float, "a_TexIndex" },
			{ ShaderDataType::Float, "a_TilingFactor" },
			{ ShaderDataType::Int, "a_EntityID" }
			});

		s_renderer_data.Quad_vertex_array = VertexArray::Create();
		s_renderer_data.Quad_vertex_array->AddVertexBuffer(Renderer2D::s_generic_renderer_data.Quad_vertex_buffer);

		s_renderer_data.Quad_vertex_array->SetIndexBuffer(s_generic_renderer_data.Quad_index_buffer);

		// Circles
		s_generic_renderer_data.Circle_vertex_buffer->SetLayout({
			{ ShaderDataType::Float3, "a_WorldPosition" },
			{ ShaderDataType::Float3, "a_LocalPosition" },
			{ ShaderDataType::Float4, "a_Color"},
			{ ShaderDataType::Float, "a_Radius" },
			{ ShaderDataType::Float, "a_Thickness" },
			{ ShaderDataType::Float, "a_Fade" },
			{ ShaderDataType::Int, "a_EntityID" }
			});

		s_renderer_data.Circle_vertex_array = VertexArray::Create();

		s_renderer_data.Circle_vertex_array->AddVertexBuffer(s_generic_renderer_data.Circle_vertex_buffer);
		s_renderer_data.Circle_vertex_array->SetIndexBuffer(s_generic_renderer_data.Quad_index_buffer);


		s_generic_renderer_data.Quad_shader->Bind();
		//s_generic_renderer_data.Circle_shader->Bind();
		
	}

	void OpenGLRenderer2D::Renderer2D_Shutdown()
	{

	}

	void OpenGLRenderer2D::Renderer2D_Flush()
	{
		if (s_generic_renderer_data.Quad_index_count != 0)
		{
			uint32_t data_size = (uint32_t)((uint8_t*)s_generic_renderer_data.Quad_vertex_buffer_ptr - (uint8_t*)s_generic_renderer_data.Quad_vertex_buffer_base_ptr);
			s_generic_renderer_data.Quad_vertex_buffer->SetData(s_generic_renderer_data.Quad_vertex_buffer_base_ptr, data_size);

			s_generic_renderer_data.Quad_shader->Bind();
			for (uint32_t i = 0; i < s_generic_renderer_data.Texture_slot_index; ++i)
			{
				s_generic_renderer_data.Texture_slots[i]->Bind(i);
			}

			RenderCommand::DrawIndexed(s_renderer_data.Quad_vertex_array, s_generic_renderer_data.Quad_index_count);
			s_generic_renderer_data.Stats.Draw_calls++;
		}

		if (s_generic_renderer_data.Circle_index_count != 0)
		{
			uint32_t data_size = (uint32_t)((uint8_t*)s_generic_renderer_data.Circle_vertex_buffer_ptr - (uint8_t*)s_generic_renderer_data.Circle_vertex_buffer_base_ptr);
			s_generic_renderer_data.Circle_vertex_buffer->SetData(s_generic_renderer_data.Circle_vertex_buffer_base_ptr, data_size);

			s_generic_renderer_data.Circle_shader->Bind();
			RenderCommand::DrawIndexed(s_renderer_data.Circle_vertex_array, s_generic_renderer_data.Circle_index_count);
			s_generic_renderer_data.Stats.Draw_calls++;
		}
	}
}
