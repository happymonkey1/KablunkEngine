#include "kablunkpch.h"

#include "Platform/Vulkan/VulkanSwapChain.h"

#include <GLFW/glfw3.h>

namespace Kablunk
{

	void VulkanSwapChain::Init(VkInstance instance, const IntrusiveRef<VulkanDevice>& device)
	{
		m_instance = instance;
		m_device = device;
	}

	void VulkanSwapChain::InitSurface(GLFWwindow* window_handle)
	{
		VkPhysicalDevice vk_physical_device = m_device->GetVkPhysicalDevice();
		
		if (glfwCreateWindowSurface(m_instance, window_handle, nullptr, &m_surface) != VK_SUCCESS)
			KB_CORE_ASSERT(false, "Failed to create Vulkan surface!");
		
		m_device->GetPhysicalDevice()->FindPresentingIndices(m_surface);
	}

	void VulkanSwapChain::Create(uint32_t* width, uint32_t* height, bool vsync)
	{
		m_vsync = vsync;

		VkDevice device = m_device->GetVkDevice();
		VkPhysicalDevice physical_device = m_device->GetVkPhysicalDevice();

		VkSwapchainKHR old_swapchain = m_swapchain;

		VkSurfaceCapabilitiesKHR surface_cap;
		if (vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, m_surface, &surface_cap) != VK_SUCCESS)
			KB_CORE_ASSERT(false, "Vulkan unable to get physical device surface capabilties!");

		uint32_t present_mode_count = 0;
		if (vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, m_surface, &present_mode_count, nullptr) != VK_SUCCESS)
			KB_CORE_ASSERT(false, "Vulkan unable to get physical device surface present mode count!");

		std::vector<VkPresentModeKHR> present_modes(present_mode_count);
		if (vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, m_surface, &present_mode_count, present_modes.data()) != VK_SUCCESS)
			KB_CORE_ASSERT(false, "Vulkan unable to get physical device surface present modes!");

		VkExtent2D swapchain_extent{};
		if (surface_cap.currentExtent.width == (uint32_t)-1)
		{
			swapchain_extent.width  = *width;
			swapchain_extent.height = *height;
		}
		else
		{
			swapchain_extent = surface_cap.currentExtent;
			*width = swapchain_extent.width;
			*height = swapchain_extent.height;
		}

		m_width = *width;
		m_height = *height;

		// select present mode for swapchain
		VkPresentModeKHR swapchain_present_mode = VK_PRESENT_MODE_FIFO_KHR;

		if (!vsync)
		{
			for (size_t i = 0; i < present_mode_count; ++i)
			{
				if (present_modes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
				{
					swapchain_present_mode = VK_PRESENT_MODE_MAILBOX_KHR;
					break;
				}
				else if ((swapchain_present_mode != VK_PRESENT_MODE_MAILBOX_KHR) && (present_modes[i] == VK_PRESENT_MODE_IMMEDIATE_KHR))
					swapchain_present_mode = VK_PRESENT_MODE_IMMEDIATE_KHR;
			}
		}

		// determine number of images
		uint32_t desired_number_of_swap_images = surface_cap.minImageCount + 1;
		if ((surface_cap.maxImageCount > 0) && (desired_number_of_swap_images > surface_cap.maxImageCount))
			desired_number_of_swap_images = surface_cap.maxImageCount;
		
		VkSurfaceTransformFlagsKHR pre_transform;
		if (surface_cap.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
			pre_transform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
		else
			pre_transform = surface_cap.currentTransform;

		// find supported composite alpha format
		VkCompositeAlphaFlagBitsKHR composite_alpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		std::vector<VkCompositeAlphaFlagBitsKHR> composite_alpha_flags = {
			VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
			VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
			VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
			VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR
		};

		for (const auto& alpha_flag : composite_alpha_flags)
		{
			if (surface_cap.supportedCompositeAlpha & alpha_flag)
			{
				composite_alpha = alpha_flag;
				break;
			}
		}

		VkSwapchainCreateInfoKHR swapchain_create_info{};
		swapchain_create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		swapchain_create_info.pNext = nullptr;
		swapchain_create_info.surface = m_surface;
		swapchain_create_info.minImageCount = desired_number_of_swap_images;
		swapchain_create_info.imageFormat = m_color_format;
		swapchain_create_info.imageColorSpace = m_color_space;
		swapchain_create_info.imageExtent = { swapchain_extent.width, swapchain_extent.height };
		swapchain_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		swapchain_create_info.preTransform = (VkSurfaceTransformFlagBitsKHR)pre_transform;
		swapchain_create_info.imageArrayLayers = 1;
		swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		swapchain_create_info.queueFamilyIndexCount = 0;
		swapchain_create_info.pQueueFamilyIndices = nullptr;
		swapchain_create_info.presentMode = swapchain_present_mode;
		swapchain_create_info.oldSwapchain = old_swapchain;
		swapchain_create_info.clipped = VK_TRUE; // discard rendering outside of surface
		swapchain_create_info.compositeAlpha = composite_alpha;

		if (surface_cap.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_SRC_BIT)
			swapchain_create_info.imageUsage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

		if (surface_cap.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT)
			swapchain_create_info.imageUsage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;

		if (vkCreateSwapchainKHR(device, &swapchain_create_info, nullptr, &m_swapchain) != VK_SUCCESS)
			KB_CORE_ASSERT(false, "Vulkan failed to create swapchain!");

		if (old_swapchain != VK_NULL_HANDLE)
		{
			for (uint32_t i = 0; i < m_image_count; ++i)
				vkDestroyImageView(device, m_buffers[i].view, nullptr);

			vkDestroySwapchainKHR(device, old_swapchain, nullptr);
		}

		if (vkGetSwapchainImagesKHR(device, m_swapchain, &m_image_count, nullptr) != VK_SUCCESS)
			KB_CORE_ERROR("Vulkan failed to retrieve swapchain image count");
		
		m_images.resize(m_image_count);
		if (vkGetSwapchainImagesKHR(device, m_swapchain, &m_image_count, m_images.data()) != VK_SUCCESS)
			KB_CORE_ERROR("Vulkan failed to retrieve swapchain images");

		// Get swap chain buffers that contain image and view
		m_buffers.resize(m_image_count);
		for (uint32_t i = 0; i < m_image_count; ++i)
		{
			VkImageViewCreateInfo color_attachment_view_create_info{};
			color_attachment_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			color_attachment_view_create_info.pNext = nullptr;
			color_attachment_view_create_info.format = m_color_format;
			color_attachment_view_create_info.components = {
				VK_COMPONENT_SWIZZLE_R,
				VK_COMPONENT_SWIZZLE_G,
				VK_COMPONENT_SWIZZLE_B,
				VK_COMPONENT_SWIZZLE_A
			};
			color_attachment_view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			color_attachment_view_create_info.subresourceRange.baseMipLevel = 0;
			color_attachment_view_create_info.subresourceRange.levelCount = 1;
			color_attachment_view_create_info.subresourceRange.baseArrayLayer = 0;
			color_attachment_view_create_info.subresourceRange.layerCount = 1;
			color_attachment_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
			color_attachment_view_create_info.flags = 0;

			m_buffers[i].image = m_images[i];

			color_attachment_view_create_info.image = m_buffers[i].image;

			if (vkCreateImageView(device, &color_attachment_view_create_info, nullptr, &m_buffers[i].view) != VK_SUCCESS)
				KB_CORE_ERROR(false, "Vulkan failed to create image view!");
		}

		// create command buffers
		VkCommandPoolCreateInfo cmd_pool_info{};
		cmd_pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		cmd_pool_info.queueFamilyIndex = m_device->GetPhysicalDevice()->GetQueueFamilyIndices().Graphics_family.value(); // #TODO could be wrong value
		cmd_pool_info.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		if (vkCreateCommandPool(device, &cmd_pool_info, nullptr, &m_command_pool) != VK_SUCCESS)
			KB_CORE_ASSERT(false, "Vulkan failed to create command pool!");

		VkCommandBufferAllocateInfo cmd_buf_allocate_info{};
		cmd_buf_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		cmd_buf_allocate_info.commandPool = m_command_pool;
		cmd_buf_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		uint32_t count = m_image_count;
		cmd_buf_allocate_info.commandBufferCount = count;
		m_command_buffers.resize(count);
		
		if (vkAllocateCommandBuffers(device, &cmd_buf_allocate_info, m_command_buffers.data()) != VK_SUCCESS)
			KB_CORE_ASSERT(false, "Vulkan failed to allocate command buffers");
	}

	void VulkanSwapChain::OnResize(uint32_t width, uint32_t height)
	{
		KB_CORE_ASSERT(false, "not implemented");
	}

	void VulkanSwapChain::BeginFrame()
	{
		KB_CORE_ASSERT(false, "not implemented");
	}

	void VulkanSwapChain::Present()
	{
		KB_CORE_ASSERT(false, "not implemented");
	}

	void VulkanSwapChain::Cleanup()
	{
		vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
	}

	VkResult VulkanSwapChain::AcquireNextImage(VkSemaphore present_complete_sem, uint32_t* image_index)
	{
		KB_CORE_ASSERT(false, "not implemented");
		return VK_SUCCESS;
	}

	VkResult VulkanSwapChain::QueuePresent(VkQueue queue, uint32_t image_index, VkSemaphore wait_sem /*= VK_NULL_HANDLE*/)
	{
		KB_CORE_ASSERT(false, "not implemented");
		return VK_SUCCESS;
	}

	void VulkanSwapChain::CreateFramebuffer()
	{
		KB_CORE_ASSERT(false, "not implemented");
	}

	void VulkanSwapChain::CreateDepthStencil()
	{
		KB_CORE_ASSERT(false, "not implemented");
	}
}
