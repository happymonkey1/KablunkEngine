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
#include "Platform/Vulkan/VulkanStorageBuffer.h"
#include "Platform/Vulkan/VulkanUniformBuffer.h"
#include "Platform/Vulkan/VulkanStorageBufferSet.h"
#include "Platform/Vulkan/VulkanUniformBufferSet.h"

#include "Kablunk/Renderer/SceneRenderer.h"
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

		std::unordered_map<UniformBufferSet*, std::unordered_map<uint64_t, std::vector<std::vector<VkWriteDescriptorSet>>>> uniform_buffer_write_descriptor_cache;
		std::unordered_map<StorageBufferSet*, std::unordered_map<uint64_t, std::vector<std::vector<VkWriteDescriptorSet>>>> storage_buffer_write_descriptor_cache;

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

		KB_CORE_INFO("creating fullscreen quad vertex buffer!");
		s_renderer_data->quad_vertex_buffer = VertexBuffer::Create(data, 4 * sizeof(QuadVertex));
		uint32_t indices[6] = { 0, 1, 2, 2, 3, 0, };
		KB_CORE_INFO("creating fullscreen quad index buffer!");
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
				vkResetDescriptorPool(vk_device, s_renderer_data->descriptor_pools[buffer_index], 0);
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
				uint32_t frame_index = Renderer::GetCurrentFrameIndex();
				VkCommandBuffer command_buffer = render_command_buffer.As<VulkanRenderCommandBuffer>()->GetCommandBuffer(frame_index);

				IntrusiveRef<VulkanPipeline> vulkan_pipeline = pipeline.As<VulkanPipeline>();

				VkPipelineLayout layout = vulkan_pipeline->GetVkPipelineLayout();

				auto vulkan_vertex_buffer = s_renderer_data->quad_vertex_buffer.As<VulkanVertexBuffer>();
				VkBuffer vk_vertex_buffer = vulkan_vertex_buffer->GetVkBuffer();
				VkDeviceSize offsets[1] = { 0 };
				vkCmdBindVertexBuffers(command_buffer, 0, 1, &vk_vertex_buffer, offsets);

				auto vulkan_index_buffer = s_renderer_data->quad_index_buffer.As<VulkanIndexBuffer>();
				VkBuffer ibBuffer = vulkan_index_buffer->GetVkBuffer();
				vkCmdBindIndexBuffer(command_buffer, ibBuffer, 0, VK_INDEX_TYPE_UINT32);

				VkPipeline pipeline = vulkan_pipeline->GetVkPipeline();
				vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

				RT_UpdateMaterialForRendering(vulkan_material, uniform_buffer_set, storage_buffer_set);

				uint32_t buffer_index = Renderer::GetCurrentFrameIndex();
				VkDescriptorSet descriptor_set = vulkan_material->GetDescriptorSet(buffer_index);
				if (descriptor_set)
					vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, layout, 0, 1, &descriptor_set, 0, nullptr);

				Buffer uniform_storage_buffer = vulkan_material->GetUniformStorageBuffer();
				if (uniform_storage_buffer.size())
					vkCmdPushConstants(command_buffer, layout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, uniform_storage_buffer.size(), uniform_storage_buffer.get());

				vkCmdDrawIndexed(command_buffer, s_renderer_data->quad_index_buffer->GetCount(), 1, 0, 0, 0);
			});

	}

	void VulkanRendererAPI::RenderGeometry(IntrusiveRef<RenderCommandBuffer> render_command_buffer, IntrusiveRef<Pipeline> pipeline, IntrusiveRef<UniformBufferSet> uniform_buffer_set, IntrusiveRef<StorageBufferSet> storage_buffer_set, IntrusiveRef<Material> material, IntrusiveRef<VertexBuffer> vertex_buffer, IntrusiveRef<IndexBuffer> index_buffer, const glm::mat4& transform, uint32_t index_count /*= 0*/)
	{
		IntrusiveRef<VulkanMaterial> vulkan_material = material.As<VulkanMaterial>();
		if (index_count == 0)
			index_count = index_buffer->GetCount();

		RenderCommand::Submit([render_command_buffer, pipeline, uniform_buffer_set, storage_buffer_set, vulkan_material, vertex_buffer, index_buffer, transform, index_count]() mutable
			{
				uint32_t frameIndex = Renderer::GetCurrentFrameIndex();
				VkCommandBuffer command_buffer = render_command_buffer.As<VulkanRenderCommandBuffer>()->GetCommandBuffer(frameIndex);

				IntrusiveRef<VulkanPipeline> vulkan_pipeline = pipeline.As<VulkanPipeline>();

				VkPipelineLayout layout = vulkan_pipeline->GetVkPipelineLayout();

				auto vulkan_geometry_vertex_buffer = vertex_buffer.As<VulkanVertexBuffer>();
				VkBuffer vk_vertex_buffer = vulkan_geometry_vertex_buffer->GetVkBuffer();
				VkDeviceSize offsets[1] = { 0 }; // wtf is this
				vkCmdBindVertexBuffers(command_buffer, 0, 1, &vk_vertex_buffer, offsets);

				auto vulkan_geometry_index_buffer = index_buffer.As<VulkanIndexBuffer>();
				VkBuffer vk_index_buffer = vulkan_geometry_index_buffer->GetVkBuffer();
				vkCmdBindIndexBuffer(command_buffer, vk_index_buffer, 0, VK_INDEX_TYPE_UINT32);

				VkPipeline vk_pipeline = vulkan_pipeline->GetVkPipeline();
				vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vk_pipeline);

				const auto& write_descriptors = RT_RetrieveOrCreateUniformBufferWriteDescriptors(uniform_buffer_set, vulkan_material);
				vulkan_material->RT_UpdateForRendering(write_descriptors);

				uint32_t buffer_index = Renderer::GetCurrentFrameIndex();
				VkDescriptorSet descriptor_set = vulkan_material->GetDescriptorSet(buffer_index);
				if (descriptor_set)
					vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, layout, 0, 1, &descriptor_set, 0, nullptr);

				vkCmdPushConstants(command_buffer, layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4), &transform);
				const Buffer& uniform_storage_buffer = vulkan_material->GetUniformStorageBuffer();
				if (uniform_storage_buffer)
					vkCmdPushConstants(command_buffer, layout, VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(glm::mat4), uniform_storage_buffer.size(), uniform_storage_buffer.get());

				vkCmdDrawIndexed(command_buffer, index_count, 1, 0, 0, 0);
			});
	}

	void VulkanRendererAPI::WaitAndRender()
	{
		RenderCommandQueue& command_queue = RenderCommand::GetRenderCommandQueue();
		command_queue.Execute();
	}

	const std::vector<std::vector<VkWriteDescriptorSet>>& VulkanRendererAPI::RT_RetrieveOrCreateUniformBufferWriteDescriptors(IntrusiveRef<UniformBufferSet> uniform_buffer_set, IntrusiveRef<VulkanMaterial> material)
	{
		size_t shader_hash = material->GetShader()->GetHash();
		if (s_renderer_data->uniform_buffer_write_descriptor_cache.find(uniform_buffer_set.get()) != s_renderer_data->uniform_buffer_write_descriptor_cache.end())
		{
			const auto& shader_map = s_renderer_data->uniform_buffer_write_descriptor_cache.at(uniform_buffer_set.get());
			if (shader_map.find(shader_hash) != shader_map.end())
			{
				const auto& write_descriptors = shader_map.at(shader_hash);
				return write_descriptors;
			}
		}

		uint32_t framesInFlight = Renderer::GetConfig().frames_in_flight;
		IntrusiveRef<VulkanShader> shader = material->GetShader().As<VulkanShader>();
		if (shader->HasDescriptorSet(0))
		{
			const auto& shader_descriptor_sets = shader->GetShaderDescriptorSets();
			if (!shader_descriptor_sets.empty())
			{
				for (auto&& [binding, shader_uniform_buffer] : shader_descriptor_sets[0].uniform_buffers)
				{
					auto& writeDescriptors = s_renderer_data->uniform_buffer_write_descriptor_cache[uniform_buffer_set.get()][shader_hash];
					writeDescriptors.resize(framesInFlight);
					for (uint32_t frame = 0; frame < framesInFlight; frame++)
					{
						IntrusiveRef<VulkanUniformBuffer> uniform_buffer = uniform_buffer_set->Get(binding, 0, frame); // set = 0 for now

						VkWriteDescriptorSet write_descriptor_set = {};
						write_descriptor_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
						write_descriptor_set.descriptorCount = 1;
						write_descriptor_set.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
						write_descriptor_set.pBufferInfo = &uniform_buffer->GetDescriptorBufferInfo();
						write_descriptor_set.dstBinding = uniform_buffer->GetBinding();
						writeDescriptors[frame].push_back(write_descriptor_set);
					}
				}

			}
		}

		return s_renderer_data->uniform_buffer_write_descriptor_cache[uniform_buffer_set.get()][shader_hash];

	}

	const std::vector<std::vector<VkWriteDescriptorSet>>& VulkanRendererAPI::RT_RetrieveOrCreateStorageBufferWriteDescriptors(IntrusiveRef<StorageBufferSet> storage_buffer_set, IntrusiveRef<VulkanMaterial> material)
	{
		size_t shader_hash = material->GetShader()->GetHash();
		if (s_renderer_data->storage_buffer_write_descriptor_cache.find(storage_buffer_set.get()) != s_renderer_data->storage_buffer_write_descriptor_cache.end())
		{
			const auto& shader_map = s_renderer_data->storage_buffer_write_descriptor_cache.at(storage_buffer_set.get());
			if (shader_map.find(shader_hash) != shader_map.end())
			{
				const auto& write_descriptors = shader_map.at(shader_hash);
				return write_descriptors;
			}
		}

		uint32_t framesInFlight = Renderer::GetConfig().frames_in_flight;
		IntrusiveRef<VulkanShader> shader = material->GetShader().As<VulkanShader>();
		if (shader->HasDescriptorSet(0))
		{
			const auto& shaderDescriptorSets = shader->GetShaderDescriptorSets();
			if (!shaderDescriptorSets.empty())
			{
				for (auto&& [binding, shader_storage_buffer] : shaderDescriptorSets[0].storage_buffers)
				{
					auto& write_descriptor = s_renderer_data->storage_buffer_write_descriptor_cache[storage_buffer_set.get()][shader_hash];
					write_descriptor.resize(framesInFlight);
					for (uint32_t frame = 0; frame < framesInFlight; ++frame)
					{
						IntrusiveRef<VulkanStorageBuffer> storage_buffer = storage_buffer_set->Get(binding, 0, frame); // set = 0 for now

						VkWriteDescriptorSet write_descriptor_set = {};
						write_descriptor_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
						write_descriptor_set.descriptorCount = 1;
						write_descriptor_set.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
						write_descriptor_set.pBufferInfo = &storage_buffer->GetVkDescriptorInfo();
						write_descriptor_set.dstBinding = storage_buffer->GetBinding();
						write_descriptor[frame].push_back(write_descriptor_set);
					}
				}
			}
		}

		return s_renderer_data->storage_buffer_write_descriptor_cache[storage_buffer_set.get()][shader_hash];
	}

	void VulkanRendererAPI::RT_UpdateMaterialForRendering(IntrusiveRef<VulkanMaterial> vulkan_material, IntrusiveRef<UniformBufferSet> uniform_buffer_set, IntrusiveRef<StorageBufferSet> storage_buffer_set)
	{
		if (uniform_buffer_set)
		{
			auto write_description = RT_RetrieveOrCreateUniformBufferWriteDescriptors(uniform_buffer_set, vulkan_material);
			if (storage_buffer_set)
			{
				const auto& storageBufferWriteDescriptors = RT_RetrieveOrCreateStorageBufferWriteDescriptors(storage_buffer_set, vulkan_material);

				const uint32_t framesInFlight = Renderer::GetConfig().frames_in_flight;
				for (uint32_t frame = 0; frame < framesInFlight; frame++)
				{
					write_description[frame].reserve(write_description[frame].size() + storageBufferWriteDescriptors[frame].size());
					write_description[frame].insert(write_description[frame].end(), storageBufferWriteDescriptors[frame].begin(), storageBufferWriteDescriptors[frame].end());
				}
			}
			vulkan_material->RT_UpdateForRendering(write_description);
		}
		else
		{
			vulkan_material->RT_UpdateForRendering();
		}
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
				viewport.minDepth = 0.0f;
				viewport.maxDepth = 1.0f;

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

				const auto& clear_values = vulkan_framebuffer->GetVkClearValues();
				render_pass_begin_info.clearValueCount = (uint32_t)clear_values.size();
				render_pass_begin_info.pClearValues = clear_values.data();

				vkCmdBeginRenderPass(cmd_buffer, &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);

				if (explicit_clear)
				{
					const uint32_t color_attachment_count = static_cast<uint32_t>(vulkan_framebuffer->GetColorAttachmentCount());
					const uint32_t total_attachment_count = color_attachment_count + (vulkan_framebuffer->HasDepthAttachment() ? 1 : 0);
					KB_CORE_ASSERT(clear_values.size() == total_attachment_count, "uh oh");

					std::vector<VkClearAttachment> attachments(total_attachment_count);
					std::vector<VkClearRect> clear_rects(total_attachment_count);
					for (uint32_t i = 0; i < color_attachment_count; i++)
					{
						attachments[i].aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
						attachments[i].colorAttachment = i;
						attachments[i].clearValue = clear_values[i];

						clear_rects[i].rect.offset = { (int32_t)0, (int32_t)0 };
						clear_rects[i].rect.extent = { width, height };
						clear_rects[i].baseArrayLayer = 0;
						clear_rects[i].layerCount = 1;
					}

					if (vulkan_framebuffer->HasDepthAttachment())
					{
						attachments[color_attachment_count].aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
						attachments[color_attachment_count].clearValue = clear_values[color_attachment_count];
						clear_rects[color_attachment_count].rect.offset = { (int32_t)0, (int32_t)0 };
						clear_rects[color_attachment_count].rect.extent = { width, height };
						clear_rects[color_attachment_count].baseArrayLayer = 0;
						clear_rects[color_attachment_count].layerCount = 1;
					}

					vkCmdClearAttachments(cmd_buffer, total_attachment_count, attachments.data(), total_attachment_count, clear_rects.data());

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
