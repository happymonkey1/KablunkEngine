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

	VkRenderPass get_vk_render_pass() const { return m_render_pass; }

	VkFramebuffer get_current_vk_framebuffer() const { return get_vk_framebuffer(m_current_image_index); }
	VkCommandBuffer get_current_vk_draw_command_buffer() const { return get_vk_draw_command_buffer(m_current_buffer_index); }

	VkFormat get_vk_color_format() const { return m_color_format; }

	u32 get_current_buffer_index() const noexcept override { return m_current_buffer_index; }

	VkFramebuffer get_vk_framebuffer(u32 index) const
    {
		KB_CORE_ASSERT(index < m_framebuffers.size(), "index out of bounds");
		return m_framebuffers[index];
	}

	VkCommandBuffer get_vk_draw_command_buffer(u32 index) const
    {
		KB_CORE_ASSERT(index < m_command_buffers.size(), "index out of bounds");
		return m_command_buffers[index].m_command_buffer;
	}

	VkSemaphore get_render_complete_semaphore() const { return m_semaphores.render_complete; }

	void destroy() noexcept override;

	u32 get_width() const noexcept override { return m_width; }
	u32 get_height() const noexcept override { return m_height; }

private:
	VkResult acquire_next_image(VkSemaphore present_complete_sem, u32* image_index);
	VkResult queue_present(VkQueue queue, u32 image_index, VkSemaphore wait_sem = VK_NULL_HANDLE) const;

	void find_image_format_and_color_space();

	void create_framebuffer();
	void create_depth_stencil();

private:
	VkInstance m_instance;
	arc<vulkan_logical_device> m_device;
	bool m_vsync = false;

	VkSwapchainKHR m_swapchain = nullptr;
	u32 m_image_count = 0;
	std::vector<VkImage> m_images;

	struct SwapChainBufferData
	{
		VkImage image;
		VkImageView view;
	};

	std::vector<SwapChainBufferData> m_buffers;

	VkFormat m_color_format;
	VkColorSpaceKHR m_color_space;

	struct DepthStencilData
	{
		VkImage image;
		VkImageView image_view;
		VmaAllocation memory_allocation;
	};

	DepthStencilData m_depth_stencil{};

	std::vector<VkFramebuffer> m_framebuffers;

    struct swapchain_command_buffer_t
    {
        VkCommandPool m_command_pool = nullptr;
        VkCommandBuffer m_command_buffer = nullptr;
    };

	std::vector<swapchain_command_buffer_t> m_command_buffers;

	struct Semaphores
	{
		VkSemaphore present_complete;
		VkSemaphore render_complete;
	};

	Semaphores m_semaphores;

	VkSubmitInfo m_submit_info;

	std::vector<VkFence> m_wait_fences;

	VkRenderPass m_render_pass;
    u32 m_current_buffer_index = 0;
    u32 m_current_image_index = 0;

    u32 m_queue_node_index = UINT32_MAX;
    u32 m_width = 0, m_height = 0;

	VkSurfaceKHR m_surface;

	friend class vulkan_context;
};

} // end namespace kb::render::backend::vk

#endif
