#include "kablunkpch.h"

#include "Kablunk/Renderer/Renderer.h"
#include "Platform/Vulkan/VulkanRendererAPI.h"

#include "Platform/Vulkan/VulkanContext.h"
#include "Platform/Vulkan/VulkanImage.h"
#include "Platform/Vulkan/VulkanRenderCommandBuffer.h"
#include "Platform/Vulkan/VulkanIndexBuffer.h"
#include "Platform/Vulkan/VulkanVertexBuffer.h"
#include "Platform/Vulkan/VulkanFramebuffer.h"
#include "Platform/Vulkan/VulkanMaterial.h"
#include "Platform/Vulkan/VulkanPipeline.h"

#include "Kablunk/Renderer/RenderCommand.h"

#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_vulkan.h"

#include <vulkan/vulkan.h>

namespace Kablunk
{

	struct VulkanRendererData
	{
		IntrusiveRef<VertexBuffer> quad_vertex_buffer;
		IntrusiveRef<IndexBuffer> quad_index_buffer;

		std::vector<VkDescriptorPool> descriptor_pools;
		std::vector<uint32_t> descriptor_pool_allocation_count;

		int32_t draw_call_count = 0;
	};

	static VulkanRendererData* s_renderer_data = nullptr;

	VulkanRendererAPI::~VulkanRendererAPI()
	{
		delete s_renderer_data;
	}

	void VulkanRendererAPI::Init()
	{
		Renderer::GetShaderLibrary()->Load("resources/shaders/Kablunk_diffuse_static.glsl");

		Renderer::GetShaderLibrary()->Load("resources/shaders/Renderer2D_Circle.glsl");
		Renderer::GetShaderLibrary()->Load("resources/shaders/Renderer2D_Quad.glsl");

		Renderer::GetShaderLibrary()->Load("resources/shaders/scene_composite.glsl");

		s_renderer_data = new VulkanRendererData{};

		VulkanAllocator::Init(VulkanContext::Get()->GetDevice());

		s_renderer_data->descriptor_pools.resize(VulkanSwapChain::MAX_FRAMES_IN_FLIGHT);
		s_renderer_data->descriptor_pool_allocation_count.resize(VulkanSwapChain::MAX_FRAMES_IN_FLIGHT);


		// Create descriptor pools
		RenderCommand::Submit([]() mutable
			{
				// Create Descriptor Pool
				VkDescriptorPoolSize pool_sizes[] =
				{
					{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
					{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
					{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
					{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
					{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
					{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
					{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
					{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
					{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
					{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
					{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
				};

				VkDescriptorPoolCreateInfo pool_info = {};
				pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
				pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
				pool_info.maxSets = 100000;
				pool_info.poolSizeCount = (uint32_t)IM_ARRAYSIZE(pool_sizes);
				pool_info.pPoolSizes = pool_sizes;
				VkDevice vk_device = VulkanContext::Get()->GetDevice()->GetVkDevice();
				uint32_t frames_in_flight = VulkanSwapChain::MAX_FRAMES_IN_FLIGHT;
				for (uint32_t i = 0; i < frames_in_flight; i++)
				{
					if (vkCreateDescriptorPool(vk_device, &pool_info, nullptr, &s_renderer_data->descriptor_pools[i]) != VK_SUCCESS)
						KB_CORE_ASSERT(false, "failed to create descriptor pools!");
					s_renderer_data->descriptor_pool_allocation_count[i] = 0;
				}
			});

		// Create fullscreen quad
		float x = -1;
		float y = -1;
		float width = 2, height = 2;
		struct QuadVertex
		{
			glm::vec3 Position;
			glm::vec2 TexCoord;
		};

		QuadVertex* data = new QuadVertex[4];

		data[0].Position = glm::vec3(x, y, 0.0f);
		data[0].TexCoord = glm::vec2(0, 0);

		data[1].Position = glm::vec3(x + width, y, 0.0f);
		data[1].TexCoord = glm::vec2(1, 0);

		data[2].Position = glm::vec3(x + width, y + height, 0.0f);
		data[2].TexCoord = glm::vec2(1, 1);

		data[3].Position = glm::vec3(x, y + height, 0.0f);
		data[3].TexCoord = glm::vec2(0, 1);

		s_renderer_data->quad_vertex_buffer = VertexBuffer::Create(data, 4 * sizeof(QuadVertex));
		uint32_t indices[6] = { 0, 1, 2, 2, 3, 0, };
		s_renderer_data->quad_index_buffer = IndexBuffer::Create(indices, 6 * sizeof(uint32_t));

		// compile shaders that were submitted
		RenderCommand::WaitAndRender();
	}

	void VulkanRendererAPI::BeginFrame()
	{
		RenderCommand::Submit([]()
			{
				VulkanSwapChain& swap_chain = VulkanContext::Get()->GetSwapchain();

				// Reset descriptor pools here
				VkDevice vk_device = VulkanContext::Get()->GetDevice()->GetVkDevice();
				uint32_t buffer_index = swap_chain.GetCurrentBufferIndex();
				vkResetDescriptorPool(vk_device, s_renderer_data->descriptor_pools [buffer_index] , 0);
				memset(s_renderer_data->descriptor_pool_allocation_count.data(), 0, s_renderer_data->descriptor_pool_allocation_count.size() * sizeof(uint32_t));

			});

	}

	void VulkanRendererAPI::EndFrame()
	{

	}

	void VulkanRendererAPI::SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
	{

	}

	void VulkanRendererAPI::SetClearColor(const glm::vec4& color)
	{

	}

	void VulkanRendererAPI::Clear()
	{

	}

	void VulkanRendererAPI::DrawIndexed(const IntrusiveRef<VertexArray>& vertexArray, uint32_t indexCount /*= 0*/)
	{

	}

	void VulkanRendererAPI::ClearImage(IntrusiveRef<RenderCommandBuffer> command_buffer, IntrusiveRef<Image2D> image)
	{
		RenderCommand::Submit([command_buffer, image = image.As<VulkanImage2D>()]
			{
				const auto vk_command_buffer = command_buffer.As<VulkanRenderCommandBuffer>()->GetCommandBuffer(Renderer::GetCurrentFrameIndex());
				VkImageSubresourceRange subresource_range{};
				subresource_range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				subresource_range.baseMipLevel = 0;
				subresource_range.levelCount = image->GetSpecification().mips;
				subresource_range.layerCount = image->GetSpecification().layers;

				VkClearColorValue clear_color{ 0.f, 0.f, 0.f, 0.f };
				vkCmdClearColorImage(vk_command_buffer, image->GetImageInfo().image, image->GetSpecification().usage == ImageUsage::Storage ? VK_IMAGE_LAYOUT_GENERAL : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, &clear_color, 1, &subresource_range);
			});
	}

	void VulkanRendererAPI::SubmitFullscreenQuad(IntrusiveRef<RenderCommandBuffer> render_command_buffer, IntrusiveRef<Pipeline> pipeline, IntrusiveRef<UniformBufferSet> uniform_buffer_set, IntrusiveRef<StorageBufferSet> storage_buffer_set, IntrusiveRef<Material> material)
	{
		IntrusiveRef<VulkanMaterial> vulkan_material = material.As<VulkanMaterial>();
		RenderCommand::Submit([render_command_buffer, pipeline, uniform_buffer_set, storage_buffer_set, vulkan_material]() mutable
			{
				uint32_t frameIndex = Renderer::GetCurrentFrameIndex();
				VkCommandBuffer commandBuffer = render_command_buffer.As<VulkanRenderCommandBuffer>()->GetCommandBuffer(frameIndex);

				IntrusiveRef<VulkanPipeline> vulkan_pipeline = pipeline.As<VulkanPipeline>();

				VkPipelineLayout layout = vulkan_pipeline->GetVkPipelineLayout();

				auto vulkan_vertex_buffer = s_renderer_data->quad_vertex_buffer.As<VulkanVertexBuffer>();
				VkBuffer vk_vertex_buffer = vulkan_vertex_buffer->GetVkBuffer();
				VkDeviceSize offsets[1] = { 0 };
				vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vk_vertex_buffer, offsets);

				auto vulkanMeshIB = s_renderer_data->quad_index_buffer.As<VulkanIndexBuffer>();
				VkBuffer ibBuffer = vulkanMeshIB->GetVkBuffer();
				vkCmdBindIndexBuffer(commandBuffer, ibBuffer, 0, VK_INDEX_TYPE_UINT32);

				VkPipeline pipeline = vulkan_pipeline->GetVkPipeline();
				vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

				//RT_UpdateMaterialForRendering(vulkanMaterial, uniformBufferSet, storageBufferSet);

				uint32_t bufferIndex = Renderer::GetCurrentFrameIndex();
				VkDescriptorSet descriptorSet = vulkan_material->GetDescriptorSet(bufferIndex);
				if (descriptorSet)
					vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, layout, 0, 1, &descriptorSet, 0, nullptr);

				Buffer uniformStorageBuffer = vulkan_material->GetUniformStorageBuffer();
				if (uniformStorageBuffer.size())
					vkCmdPushConstants(commandBuffer, layout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, uniformStorageBuffer.size(), uniformStorageBuffer.get());

				vkCmdDrawIndexed(commandBuffer, s_renderer_data->quad_index_buffer->GetCount(), 1, 0, 0, 0);
			});

	}

	void VulkanRendererAPI::WaitAndRender()
	{
		RenderCommandQueue& command_queue = RenderCommand::GetRenderCommandQueue();
		command_queue.Execute();
	}

	VkDescriptorSet VulkanRendererAPI::RT_AllocateDescriptorSet(VkDescriptorSetAllocateInfo& alloc_info)
	{
		VkDevice device = VulkanContext::Get()->GetDevice()->GetVkDevice();
		uint32_t buffer_index = Renderer::GetCurrentFrameIndex();
		alloc_info.descriptorPool = s_renderer_data->descriptor_pools[buffer_index];
		
		VkDescriptorSet descriptor_set;
		if (vkAllocateDescriptorSets(device, &alloc_info, &descriptor_set) != VK_SUCCESS)
			KB_CORE_ASSERT(false, "Vulkan failed to allocate descriptor set!");

		s_renderer_data->descriptor_pool_allocation_count[buffer_index] += alloc_info.descriptorSetCount;
		return descriptor_set;
	}

	void VulkanRendererAPI::BeginRenderPass(IntrusiveRef<RenderCommandBuffer> render_command_buffer, const IntrusiveRef<RenderPass>& render_pass, bool explicit_clear)
	{
		RenderCommand::Submit([render_command_buffer, render_pass, explicit_clear]()
			{
				uint32_t frame_index = Renderer::GetCurrentFrameIndex();
				VkCommandBuffer cmd_buffer = render_command_buffer.As<VulkanRenderCommandBuffer>()->GetCommandBuffer(frame_index);

				auto framebuffer = render_pass->GetSpecification().target_framebuffer;
				IntrusiveRef<VulkanFramebuffer> vulkan_framebuffer = framebuffer.As<VulkanFramebuffer>();
				const auto& framebuffer_spec = vulkan_framebuffer->GetSpecification();

				uint32_t width = framebuffer_spec.width;
				uint32_t height = framebuffer_spec.height;

				VkViewport viewport{};
				viewport.width = width;
				viewport.height = height;

				VkRenderPassBeginInfo render_pass_begin_info{};
				render_pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
				render_pass_begin_info.pNext = nullptr;
				render_pass_begin_info.renderPass = vulkan_framebuffer->GetVkRenderPass();
				render_pass_begin_info.renderArea.offset.x = 0;
				render_pass_begin_info.renderArea.offset.y = 0;
				render_pass_begin_info.renderArea.extent.width = width;
				render_pass_begin_info.renderArea.extent.height = height;
				if (framebuffer_spec.swap_chain_target)
				{
					VulkanSwapChain& swap_chain = VulkanContext::Get()->GetSwapchain();
					width = swap_chain.GetWidth();
					height = swap_chain.GetHeight();
					render_pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
					render_pass_begin_info.pNext = nullptr;
					render_pass_begin_info.renderPass = vulkan_framebuffer->GetVkRenderPass();
					render_pass_begin_info.renderArea.offset.x = 0;
					render_pass_begin_info.renderArea.offset.y = 0;
					render_pass_begin_info.renderArea.extent.width = width;
					render_pass_begin_info.renderArea.extent.height = height;
					render_pass_begin_info.framebuffer = swap_chain.GetCurrentFramebuffer();

					viewport.x = 0.0f;
					viewport.y = static_cast<float>(height);
					viewport.width = static_cast<float>(width);
					viewport.height = -static_cast<float>(height);
				}
				else
				{
					width = framebuffer->GetWidth();
					height = framebuffer->GetHeight();
					render_pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
					render_pass_begin_info.pNext = nullptr;
					render_pass_begin_info.renderPass = vulkan_framebuffer->GetVkRenderPass();
					render_pass_begin_info.renderArea.offset.x = 0;
					render_pass_begin_info.renderArea.offset.y = 0;
					render_pass_begin_info.renderArea.extent.width = width;
					render_pass_begin_info.renderArea.extent.height = height;
					render_pass_begin_info.framebuffer = vulkan_framebuffer->GetVkFramebuffer();

					viewport.x = 0.0f;
					viewport.y = 0.0f;
					viewport.width = static_cast<float>(width);
					viewport.height = static_cast<float>(height);
				}

				const auto& clearValues = vulkan_framebuffer->GetVkClearValues();
				render_pass_begin_info.clearValueCount = (uint32_t)clearValues.size();
				render_pass_begin_info.pClearValues = clearValues.data();

				vkCmdBeginRenderPass(cmd_buffer, &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);

				if (explicit_clear)
				{
					const uint32_t colorAttachmentCount = static_cast<uint32_t>(vulkan_framebuffer->GetColorAttachmentCount());
					const uint32_t totalAttachmentCount = colorAttachmentCount + (vulkan_framebuffer->HasDepthAttachment() ? 1 : 0);
					KB_CORE_ASSERT(clearValues.size() == totalAttachmentCount, "uh oh");

					std::vector<VkClearAttachment> attachments(totalAttachmentCount);
					std::vector<VkClearRect> clearRects(totalAttachmentCount);
					for (uint32_t i = 0; i < colorAttachmentCount; i++)
					{
						attachments[i].aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
						attachments[i].colorAttachment = i;
						attachments[i].clearValue = clearValues[i];

						clearRects[i].rect.offset = { (int32_t)0, (int32_t)0 };
						clearRects[i].rect.extent = { width, height };
						clearRects[i].baseArrayLayer = 0;
						clearRects[i].layerCount = 1;
					}

					if (vulkan_framebuffer->HasDepthAttachment())
					{
						attachments[colorAttachmentCount].aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
						attachments[colorAttachmentCount].clearValue = clearValues[colorAttachmentCount];
						clearRects[colorAttachmentCount].rect.offset = { (int32_t)0, (int32_t)0 };
						clearRects[colorAttachmentCount].rect.extent = { width, height };
						clearRects[colorAttachmentCount].baseArrayLayer = 0;
						clearRects[colorAttachmentCount].layerCount = 1;
					}

					vkCmdClearAttachments(cmd_buffer, totalAttachmentCount, attachments.data(), totalAttachmentCount, clearRects.data());

				}

				// Update dynamic viewport state
				vkCmdSetViewport(cmd_buffer, 0, 1, &viewport);

				// Update dynamic scissor state
				VkRect2D scissor = {};
				scissor.extent.width = width;
				scissor.extent.height = height;
				scissor.offset.x = 0;
				scissor.offset.y = 0;
				vkCmdSetScissor(cmd_buffer, 0, 1, &scissor);

			});
	}

	void VulkanRendererAPI::EndRenderPass(IntrusiveRef<RenderCommandBuffer> render_command_buffer)
	{
		RenderCommand::Submit([render_command_buffer]()
			{
				uint32_t frame_index = Renderer::GetCurrentFrameIndex();
				VkCommandBuffer vk_command_buffer = render_command_buffer.As<VulkanRenderCommandBuffer>()->GetCommandBuffer(frame_index);

				vkCmdEndRenderPass(vk_command_buffer);
			});
	}
}
