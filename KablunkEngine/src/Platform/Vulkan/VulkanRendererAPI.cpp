#include "kablunkpch.h"

#include "Kablunk/Renderer/Renderer.h"
#include "Platform/Vulkan/VulkanRendererAPI.h"

#include "Platform/Vulkan/VulkanContext.h"
#include "Platform/Vulkan/VulkanImage.h"
#include "Platform/Vulkan/VulkanRenderCommandBuffer.h"
#include "Platform/Vulkan/VulkanIndexBuffer.h"
#include "Platform/Vulkan/VulkanVertexBuffer.h"

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


	void VulkanRendererAPI::Init()
	{
		Renderer::GetShaderLibrary()->Load("resources/shaders/Kablunk_diffuse_static.glsl");

		Renderer::GetShaderLibrary()->Load("resources/shaders/Renderer2D_Circle.glsl");
		Renderer::GetShaderLibrary()->Load("resources/shaders/Renderer2D_Quad.glsl");

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

		KB_CORE_INFO("Vulkan Renderer creating vertex buffer");
		s_renderer_data->quad_vertex_buffer = VertexBuffer::Create(data, 4 * sizeof(QuadVertex));
		uint32_t indices[6] = { 0, 1, 2, 2, 3, 0, };
		KB_CORE_INFO("Vulkan Renderer creating index buffer");
		s_renderer_data->quad_index_buffer = IndexBuffer::Create(indices, 6 * sizeof(uint32_t));



		// compile shaders that were submitted
		KB_CORE_INFO("Vulkan Renderer about to compile shaders!");
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

}
