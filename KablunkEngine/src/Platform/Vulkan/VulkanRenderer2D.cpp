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
	}

	void VulkanRenderer2D::Renderer2D_Shutdown()
	{
		KB_CORE_ASSERT(false, "not implemented!");
	}

	void VulkanRenderer2D::Renderer2D_Flush()
	{
		if (!m_scene_renderer)
			return;

		uint32_t frame_index = Renderer::GetCurrentFrameIndex();
		VkCommandBuffer vk_command_buffer = m_scene_renderer->m_command_buffer.As<VulkanRenderCommandBuffer>()->GetCommandBuffer(frame_index);

		// Quads
		if (s_generic_renderer_data.Quad_index_count != 0)
		{
			m_vulkan_renderer->BeginRenderPass(m_scene_renderer->m_command_buffer, m_scene_renderer->m_quad_pipeline->GetSpecification().render_pass);

			uint32_t data_size = (uint32_t)((uint8_t*)s_generic_renderer_data.Quad_vertex_buffer_ptr - (uint8_t*)s_generic_renderer_data.Quad_vertex_buffer_base_ptr);
			s_generic_renderer_data.Quad_vertex_buffer->SetData(s_generic_renderer_data.Quad_vertex_buffer_base_ptr, data_size);

			// Bind vertex buffer
			IntrusiveRef<VulkanVertexBuffer> vertex_buffer = s_generic_renderer_data.Quad_vertex_buffer.As<VulkanVertexBuffer>();
			VkBuffer vk_vertex_buffer = vertex_buffer->GetVkBuffer();

			VkDeviceSize offsets[1] = { 0 }; // wtf is this?
			vkCmdBindVertexBuffers(vk_command_buffer, 0, 1, &vk_vertex_buffer, offsets);

			// Bind index buffer
			IntrusiveRef<VulkanIndexBuffer> index_buffer = s_generic_renderer_data.Quad_index_buffer.As<VulkanIndexBuffer>();
			VkBuffer vk_index_buffer = vertex_buffer->GetVkBuffer();
			vkCmdBindIndexBuffer(vk_command_buffer, vk_index_buffer, 0, VK_INDEX_TYPE_UINT32);

			// Bind pipeline
			VkPipeline vk_pipeline = m_scene_renderer->m_quad_pipeline.As<VulkanPipeline>()->GetVkPipeline();
			vkCmdBindPipeline(vk_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vk_pipeline);

			//vkCmdPushConstants(vk_command_buffer, layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4), &transform);
			//vkCmdPushConstants(vk_command_buffer, layout, VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(glm::mat4), uniformStorageBuffer.Size, uniformStorageBuffer.Data);

			vkCmdDrawIndexed(vk_command_buffer, s_generic_renderer_data.Quad_index_buffer->GetCount(), 1, 0, 0, 0);
			s_generic_renderer_data.Stats.Draw_calls++;

			m_vulkan_renderer->EndRenderPass(m_scene_renderer->m_command_buffer);
		}

		// Circles
		if (s_generic_renderer_data.Circle_index_count != 0)
		{
			m_vulkan_renderer->BeginRenderPass(m_scene_renderer->m_command_buffer, m_scene_renderer->m_circle_pipeline->GetSpecification().render_pass);

			uint32_t data_size = (uint32_t)((uint8_t*)s_generic_renderer_data.Circle_vertex_buffer_ptr - (uint8_t*)s_generic_renderer_data.Circle_vertex_buffer_base_ptr);
			s_generic_renderer_data.Circle_vertex_buffer->SetData(s_generic_renderer_data.Circle_vertex_buffer_base_ptr, data_size);

			// Bind vertex buffer
			IntrusiveRef<VulkanVertexBuffer> vertex_buffer = s_generic_renderer_data.Circle_vertex_buffer.As<VulkanVertexBuffer>();
			VkBuffer vk_vertex_buffer = vertex_buffer->GetVkBuffer();

			VkDeviceSize offsets[1] = { 0 }; // wtf is this?
			vkCmdBindVertexBuffers(vk_command_buffer, 0, 1, &vk_vertex_buffer, offsets);

			// Bind index buffer
			IntrusiveRef<VulkanIndexBuffer> index_buffer = s_generic_renderer_data.Circle_index_buffer.As<VulkanIndexBuffer>();
			VkBuffer vk_index_buffer = vertex_buffer->GetVkBuffer();
			vkCmdBindIndexBuffer(vk_command_buffer, vk_index_buffer, 0, VK_INDEX_TYPE_UINT32);

			// Bind pipeline
			VkPipeline vk_pipeline = m_scene_renderer->m_circle_pipeline.As<VulkanPipeline>()->GetVkPipeline();
			vkCmdBindPipeline(vk_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vk_pipeline);

			//vkCmdPushConstants(vk_command_buffer, layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4), &transform);
			//vkCmdPushConstants(vk_command_buffer, layout, VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(glm::mat4), uniformStorageBuffer.Size, uniformStorageBuffer.Data);

			vkCmdDrawIndexed(vk_command_buffer, s_generic_renderer_data.Circle_index_buffer->GetCount(), 1, 0, 0, 0);
			s_generic_renderer_data.Stats.Draw_calls++;

			m_vulkan_renderer->EndRenderPass(m_scene_renderer->m_command_buffer);
		}
	}

}
