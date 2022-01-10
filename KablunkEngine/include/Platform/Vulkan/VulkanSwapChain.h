#pragma once
#ifndef KABLUNK_PLATFORM_VULKAN_SWAPCHAIN_H
#define KABLUNK_PLATFORM_VULKAN_SWAPCHAIN_H

#include "Kablunk/Core/Core.h"
#include "Platform/Vulkan/VulkanDevice.h"

#include <vulkan/vulkan.h>

struct GLFWwindow;

namespace Kablunk
{
	class VulkanSwapChain
	{
	public:
		VulkanSwapChain() = default;

		void Init(VkInstance instance, const IntrusiveRef<VulkanDevice>& device);
		void InitSurface(GLFWwindow* window_handle);
		void Create(uint32_t* width, uint32_t* height, bool vsync);

		void OnResize(uint32_t width, uint32_t height);

		void BeginFrame();
		void Present();

		uint32_t GetImageCount() const { return m_image_count; }

		VkFramebuffer GetCurrentFrameBuffer() { return GetFramebuffer(m_current_image_index); }
		VkCommandBuffer GetCurrentDrawCommandBuffer() { return GetDrawCommandBuffer(m_current_buffer_index); }

		uint32_t GetCurrentBufferIndex() const { return m_current_buffer_index; }
		VkFramebuffer GetFramebuffer(uint32_t index)
		{
			KB_CORE_ASSERT(index < m_framebuffers.size(), "index out of bounds");
			return m_framebuffers[index];
		}

		VkCommandBuffer GetDrawCommandBuffer(uint32_t index)
		{
			KB_CORE_ASSERT(index < m_command_buffers.size(), "index out of bounds");
			return m_command_buffers[index];
		}

		VkSemaphore GetRenderCompleteSemaphore() { return m_semaphores.RenderComplete; }

		void Cleanup();
	private:
		VkResult AcquireNextImage(VkSemaphore present_complete_sem, uint32_t* image_index);
		VkResult QueuePresent(VkQueue queue, uint32_t image_index, VkSemaphore wait_sem = VK_NULL_HANDLE);

		void CreateFramebuffer();
		void CreateDepthStencil();
	private:
		VkInstance m_instance;
		IntrusiveRef<VulkanDevice> m_device;
		bool m_vsync = false;

		VkSwapchainKHR m_swaphchain = nullptr;
		uint32_t m_image_count = 0;
		std::vector<VkImage> m_images;

		struct SwapChainBufferData
		{
			VkImage image;
			VkImageView view;
		};
		std::vector<SwapChainBufferData> m_buffers;

		struct DepthStencilData
		{
			VkImage image;
			// #TODO custom allocator
			VkImageView view;
		};
		DepthStencilData m_depth_stencil;
		
		std::vector<VkFramebuffer> m_framebuffers;
		VkCommandPool m_command_pool = nullptr;
		std::vector<VkCommandBuffer> m_command_buffers;

		struct Semaphores
		{
			VkSemaphore PresentComplete;
			VkSemaphore RenderComplete;
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

		friend class VulkanContext;
	};
}

#endif
