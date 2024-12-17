#pragma once
#ifndef KABLUNK_RENDERER_BACKEND_VULKAN_SWAP_CHAIN_H
#define KABLUNK_RENDERER_BACKEND_VULKAN_SWAP_CHAIN_H

#include "Kablunk/Core/Core.h"
#include "kablunk/renderer/backend/vulkan/vulkan_logical_device.h"
#include "kablunk/renderer/backend/vulkan/vulkan_allocator.h"
#include "Kablunk/renderer/backend/swap_chain.h"

#include <vulkan/vulkan.h>


struct GLFWwindow;

namespace kb::render::backend::vk
{ // start namespace kb::render::backend::vk

class vulkan_swap_chain : public swap_chain
{
public:
	vulkan_swap_chain() = default;
    ~vulkan_swap_chain() noexcept = default;

	void init(VkInstance instance, const arc<vulkan_logical_device>& device);
	void init_surface(GLFWwindow* window_handle) noexcept override;
	void create(u32* width, u32* height, bool vsync) noexcept override;

	void on_resize(u32 width, u32 height) noexcept override;

	void begin_frame() noexcept override;
	void present() noexcept override;

    u32 get_image_count() const { return m_image_count; }

	VkRenderPass get_vk_render_pass() const { return m_vk_render_pass; }

	VkFramebuffer get_current_vk_frame_buffer() const { return get_vk_frame_buffer(m_current_image_index); }
	VkCommandBuffer get_current_vk_draw_command_buffer() const { return get_vk_draw_command_buffer(m_current_frame_index); }

	VkFormat get_vk_color_format() const { return m_vk_color_format; }

	u32 get_current_buffer_index() const noexcept override { return m_current_frame_index; }

	VkFramebuffer get_vk_frame_buffer(u32 index) const
    {
		KB_CORE_ASSERT(index < m_frame_buffers.size(), "index out of bounds");
		return m_frame_buffers[index];
	}

	VkCommandBuffer get_vk_draw_command_buffer(u32 index) const
    {
		KB_CORE_ASSERT(index < m_command_buffers.size(), "index out of bounds");
		return m_command_buffers[index].m_command_buffer;
	}

	VkSemaphore get_render_complete_semaphore() const { return m_semaphores.m_render_complete_semaphores[m_current_frame_index]; }

	void destroy() noexcept override;

	u32 get_width() const noexcept override { return m_width; }
	u32 get_height() const noexcept override { return m_height; }

private:
	VkResult acquire_next_image(VkSemaphore present_complete_sem, u32* image_index);
	VkResult queue_present(VkQueue queue, u32 image_index, VkSemaphore wait_sem = VK_NULL_HANDLE) const;

	void find_image_format_and_color_space();

	void create_frame_buffer();
	void create_depth_stencil();

private:
	VkInstance m_instance;
	arc<vulkan_logical_device> m_device;
	bool m_vsync = false;

	VkSwapchainKHR m_vk_swap_chain = nullptr;
	u32 m_image_count = 0;
	std::vector<VkImage> m_vk_images;

	struct SwapChainBufferData
	{
		VkImage m_vk_image;
		VkImageView m_vk_image_view;
	};

	std::vector<SwapChainBufferData> m_buffers;

	VkFormat m_vk_color_format;
	VkColorSpaceKHR m_vk_color_space;

	struct DepthStencilData
	{
		VkImage m_vk_image;
		VkImageView m_vk_image_view;
		VmaAllocation m_vk_memory_allocation;
	};

	DepthStencilData m_depth_stencil{};

	std::vector<VkFramebuffer> m_frame_buffers;

    struct swapchain_command_buffer_t
    {
        VkCommandPool m_command_pool = nullptr;
        VkCommandBuffer m_command_buffer = nullptr;
    };

	std::vector<swapchain_command_buffer_t> m_command_buffers;

    struct semaphores_t
    {
        // Semaphore to signal image is available for each frame in flight
        std::vector<VkSemaphore> m_image_available_semaphores;
        // Semaphore to signal image has finished rendered for each frame in flight
        std::vector<VkSemaphore> m_render_complete_semaphores;
    } m_semaphores;

	std::vector<VkFence> m_wait_fences;

	VkRenderPass m_vk_render_pass;
    u32 m_current_frame_index = 0;
    u32 m_current_image_index = 0;

    u32 m_queue_node_index = UINT32_MAX;
    u32 m_width = 0, m_height = 0;

	VkSurfaceKHR m_vk_surface;

	friend class vulkan_context;
};

} // end namespace kb::render::backend::vk

#endif
