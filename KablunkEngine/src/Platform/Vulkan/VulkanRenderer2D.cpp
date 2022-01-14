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
		m_command_buffer = RenderCommandBuffer::CreateFromSwapChain("VulkanRenderer2D");

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
			pipeline_spec.shader = s_generic_renderer_data.Quad_shader;
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
			pipeline_spec.shader = s_generic_renderer_data.Circle_shader;
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

	}

	void VulkanRenderer2D::Renderer2D_Shutdown()
	{
		KB_CORE_ASSERT(false, "not implemented!");
	}

	void VulkanRenderer2D::Renderer2D_Flush()
	{
		//m_command_buffer->BeginTimestampQuery();
		
		IntrusiveRef<VulkanRenderCommandBuffer> command_buffer = m_command_buffer.As<VulkanRenderCommandBuffer>();
		IntrusiveRef<VulkanPipeline> quad_pipeline = m_quad_pipeline.As<VulkanPipeline>();
		IntrusiveRef<VulkanPipeline> circle_pipeline = m_circle_pipeline.As<VulkanPipeline>();

		VulkanRendererAPI* vulkan_renderer = dynamic_cast<VulkanRendererAPI*>(RenderCommand::GetRenderer());

		uint32_t frame_index = Renderer::GetCurrentFrameIndex();
		VkCommandBuffer vk_command_buffer = command_buffer->GetCommandBuffer(frame_index);

		// Quads
		if (s_generic_renderer_data.Quad_index_count != 0)
		{
			vulkan_renderer->BeginRenderPass(command_buffer, quad_pipeline->GetSpecification().render_pass);
			RenderCommand::Submit([vk_command_buffer, quad_pipeline, circle_pipeline]() mutable
				{
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
					VkPipeline vk_pipeline = quad_pipeline->GetVkPipeline();
					vkCmdBindPipeline(vk_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vk_pipeline);

					//vkCmdPushConstants(vk_command_buffer, layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4), &transform);
					//vkCmdPushConstants(vk_command_buffer, layout, VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(glm::mat4), uniformStorageBuffer.Size, uniformStorageBuffer.Data);

					vkCmdDrawIndexed(vk_command_buffer, s_generic_renderer_data.Quad_index_buffer->GetCount(), 1, 0, 0, 0);
					s_generic_renderer_data.Stats.Draw_calls++;
				});
			vulkan_renderer->EndRenderPass(command_buffer);
		}

		
		// Circles
		if (s_generic_renderer_data.Circle_index_count != 0)
		{
			vulkan_renderer->BeginRenderPass(command_buffer, circle_pipeline->GetSpecification().render_pass);
			RenderCommand::Submit([vk_command_buffer, quad_pipeline, circle_pipeline]() mutable
				{
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
					VkPipeline vk_pipeline = circle_pipeline->GetVkPipeline();
					vkCmdBindPipeline(vk_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vk_pipeline);

					//vkCmdPushConstants(vk_command_buffer, layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4), &transform);
					//vkCmdPushConstants(vk_command_buffer, layout, VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(glm::mat4), uniformStorageBuffer.Size, uniformStorageBuffer.Data);

					vkCmdDrawIndexed(vk_command_buffer, s_generic_renderer_data.Circle_index_buffer->GetCount(), 1, 0, 0, 0);
					s_generic_renderer_data.Stats.Draw_calls++;
				});
			vulkan_renderer->EndRenderPass(command_buffer);
		}


		//m_command_buffer->EndTimestampQuery();
	}

}
