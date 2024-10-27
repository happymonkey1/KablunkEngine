#pragma once
#ifndef KABLUNK_RENDERER_BACKEND_VULKAN_SWAPCHAIN_H
#define KABLUNK_RENDERER_BACKEND_VULKAN_SWAPCHAIN_H

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
    ~vulkan_swap_chain() noexcept override = default;

	void Init(VkInstance instance, const arc<vulkan_logical_device>& device);
	void init_surface(GLFWwindow* window_handle) noexcept override;
	void create(uint32_t* width, uint32_t* height, bool vsync) noexcept override;

	void OnResize(uint32_t width, uint32_t height) noexcept override;

	void BeginFrame() noexcept override;
	void Present() noexcept override;

	uint32_t GetImageCount() const { return m_image_count; }

	VkRenderPass get_vk_render_pass() const { return m_render_pass; }

	VkFramebuffer GetCurrentFramebuffer() const { return GetFramebuffer(m_current_image_index); }
	VkCommandBuffer GetCurrentDrawCommandBuffer() const { return GetDrawCommandBuffer(m_current_buffer_index); }

	VkFormat GetColorFormat() const { return m_color_format; }

	u32 GetCurrentBufferIndex() const noexcept override { return m_current_buffer_index; }

	VkFramebuffer GetFramebuffer(uint32_t index) const
    {
		KB_CORE_ASSERT(index < m_framebuffers.size(), "index out of bounds");
		return m_framebuffers[index];
	}

	VkCommandBuffer GetDrawCommandBuffer(uint32_t index) const
    {
		KB_CORE_ASSERT(index < m_command_buffers.size(), "index out of bounds");
		return m_command_buffers[index].m_command_buffer;
	}

	VkSemaphore GetRenderCompleteSemaphore() const { return m_semaphores.render_complete; }

	void Destroy() noexcept override;

	u32 GetWidth() const noexcept override { return m_width; }
	u32 GetHeight() const noexcept override { return m_height; }
private:
	VkResult AcquireNextImage(VkSemaphore present_complete_sem, uint32_t* image_index);
	VkResult QueuePresent(VkQueue queue, uint32_t image_index, VkSemaphore wait_sem = VK_NULL_HANDLE) const;

	void FindImageFormatAndColorSpace();

	void CreateFramebuffer();
	void CreateDepthStencil();
private:
	VkInstance m_instance;
	arc<vulkan_logical_device> m_device;
	bool m_vsync = false;

	VkSwapchainKHR m_swapchain = nullptr;
	uint32_t m_image_count = 0;
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
	uint32_t m_current_buffer_index = 0;
	uint32_t m_current_image_index = 0;

	uint32_t m_queue_node_index = UINT32_MAX;
	uint32_t m_width = 0, m_height = 0;

	VkSurfaceKHR m_surface;

	friend class vulkan_context;
};

} // end namespace kb::render::backend::vk

#endif
