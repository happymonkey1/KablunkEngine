#include "kablunkpch.h"

#include "Platform/Vulkan/VulkanRenderer2D.h"
#include "Kablunk/Renderer/RenderCommand.h"

#include <Platform/Vulkan/VulkanVertexBuffer.h>
#include <Platform/Vulkan/VulkanIndexBuffer.h>
#include <Platform/Vulkan/VulkanRendererAPI.h>

#include <vulkan/vulkan.h>

namespace Kablunk
{

	void VulkanRenderer2D::Renderer2D_Init()
	{
		VulkanRendererAPI* m_vulkan_renderer = dynamic_cast<VulkanRendererAPI*>(RenderCommand::GetRenderer());

		m_quad_material = Material::Create(s_generic_renderer_data.Quad_shader);
		m_circle_material = Material::Create(s_generic_renderer_data.Circle_shader);

		//m_render_command_buffer = RenderCommandBuffer::Create(0, "Renderer2D");
		m_render_command_buffer = RenderCommandBuffer::Create(0, "Renderer2D");
	}

	void VulkanRenderer2D::Renderer2D_Shutdown()
	{
		KB_CORE_ASSERT(false, "not implemented!");
	}

	void VulkanRenderer2D::Renderer2D_SetSceneRenderer(IntrusiveRef<SceneRenderer> scene_renderer)
	{
		m_scene_renderer = scene_renderer;
	}

	void VulkanRenderer2D::Renderer2D_Flush()
	{
		if (!m_scene_renderer)
			return;

		// Quads
		m_render_command_buffer->Begin();
		m_vulkan_renderer->BeginRenderPass(m_render_command_buffer, m_scene_renderer->m_quad_pipeline->GetSpecification().render_pass);
		if (s_generic_renderer_data.Quad_index_count != 0)
		{
			uint32_t data_size = (uint32_t)((uint8_t*)s_generic_renderer_data.Quad_vertex_buffer_ptr - (uint8_t*)s_generic_renderer_data.Quad_vertex_buffer_base_ptr);
			s_generic_renderer_data.Quad_vertex_buffer->SetData(s_generic_renderer_data.Quad_vertex_buffer_base_ptr, data_size);

			// Set Textures
			auto& textures = s_generic_renderer_data.Texture_slots;
			for (uint32_t i = 0; i < s_generic_renderer_data.Texture_slot_index; i++)
			{
				if (textures[i])
					m_quad_material->Set("u_Textures", textures[i], i);
				else
					m_quad_material->Set("u_Textures", s_generic_renderer_data.White_texture, i);
			}

			RenderCommand::RenderGeometry(m_render_command_buffer, m_scene_renderer->m_quad_pipeline, m_scene_renderer->m_uniform_buffer_set, nullptr, m_quad_material, s_generic_renderer_data.Quad_vertex_buffer, s_generic_renderer_data.Quad_index_buffer, glm::mat4{ 1.0f }, s_generic_renderer_data.Quad_index_count);
			s_generic_renderer_data.Stats.Draw_calls++;
		}
		m_vulkan_renderer->EndRenderPass(m_render_command_buffer);

		// Circles
		m_vulkan_renderer->BeginRenderPass(m_render_command_buffer, m_scene_renderer->m_circle_pipeline->GetSpecification().render_pass);
		if (s_generic_renderer_data.Circle_index_count != 0)
		{
			// calculate data size in bytes
			uint32_t data_size = (uint32_t)((uint8_t*)s_generic_renderer_data.Circle_vertex_buffer_ptr - (uint8_t*)s_generic_renderer_data.Circle_vertex_buffer_base_ptr);
			s_generic_renderer_data.Circle_vertex_buffer->SetData(s_generic_renderer_data.Circle_vertex_buffer_base_ptr, data_size);

			RenderCommand::RenderGeometry(m_render_command_buffer, m_scene_renderer->m_circle_pipeline, m_scene_renderer->m_uniform_buffer_set, nullptr, m_circle_material, s_generic_renderer_data.Circle_vertex_buffer, s_generic_renderer_data.Circle_index_buffer, glm::mat4{ 1.0f }, s_generic_renderer_data.Circle_index_count);

			s_generic_renderer_data.Stats.Draw_calls++;
		}
		m_vulkan_renderer->EndRenderPass(m_render_command_buffer);
		m_render_command_buffer->End();
		m_render_command_buffer->Submit();
	}

}
