#include "kablunkpch.h"

#include "Platform/Vulkan/VulkanSwapChain.h"

#include "Kablunk/Renderer/Renderer.h"
#include "Kablunk/Renderer/RenderCommand.h"

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

		FindImageFormatAndColorSpace();
	}

	void VulkanSwapChain::Create(uint32_t* width, uint32_t* height, bool vsync)
	{
		KB_CORE_INFO("Creating VulkanSwapChain!");
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
		if (surface_cap.currentExtent.width == 0xFFFFFFFF)
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

		// Create swapchain
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

		// destroy old swap chain if it exists, since we are recreating the swapchain.
		// also cleans up old presenting images
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
				KB_CORE_ASSERT(false, "Vulkan failed to create image view!");
		}

		// create command buffers
		KB_CORE_ASSERT(m_device->GetPhysicalDevice()->GetQueueFamilyIndices().Graphics_family.has_value(), "graphics family queue has no index set!");
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

		// Create semaphores
		VkSemaphoreCreateInfo semaphore_create_info{};
		semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		
		if (vkCreateSemaphore(device, &semaphore_create_info, nullptr, &m_semaphores.present_complete) != VK_SUCCESS)
			KB_CORE_ASSERT(false, "Vulkan failed to create presenting semaphore!");

		if (vkCreateSemaphore(device, &semaphore_create_info, nullptr, &m_semaphores.render_complete) != VK_SUCCESS)
			KB_CORE_ASSERT(false, "Vulkan failed to create rendering semaphore!");

		VkPipelineStageFlags pipeline_stage_flags = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

		// setup submit info struct
		m_submit_info = {};
		m_submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		m_submit_info.pWaitDstStageMask = &pipeline_stage_flags;
		m_submit_info.waitSemaphoreCount = 1;
		m_submit_info.pWaitSemaphores = &m_semaphores.present_complete;
		m_submit_info.signalSemaphoreCount = 1;
		m_submit_info.pSignalSemaphores = &m_semaphores.render_complete;

		// Wait for fences to sync command buffer access
		VkFenceCreateInfo fence_create_info{};
		fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fence_create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		
		m_wait_fences.resize(render::get_frames_in_flights());
		for (auto& fence : m_wait_fences)
			if (vkCreateFence(device, &fence_create_info, nullptr, &fence) != VK_SUCCESS)
				KB_CORE_ASSERT(false, "Vulkan failed to create fence!");

		// Swapchain doesn't need depth stencil?
		//CreateDepthStencil();

		// render pass
		VkFormat depth_format = m_device->GetPhysicalDevice()->GetDepthFormat();

		std::array<VkAttachmentDescription, 2> attachments{};
		// Color attachment
		attachments[0].format = m_color_format;
		attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
		attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		attachments[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		// Depth attachment
		attachments[1].format = depth_format;
		attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
		attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkAttachmentReference color_reference = {};
		color_reference.attachment = 0;
		color_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentReference depth_reference = {};
		depth_reference.attachment = 1;
		depth_reference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass_description = {};
		subpass_description.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass_description.colorAttachmentCount = 1;
		subpass_description.pColorAttachments = &color_reference;
		//subpass_description.pDepthStencilAttachment = &depth_reference;
		subpass_description.inputAttachmentCount = 0;
		subpass_description.pInputAttachments = nullptr;
		subpass_description.preserveAttachmentCount = 0;
		subpass_description.pPreserveAttachments = nullptr;
		subpass_description.pResolveAttachments = nullptr;

		VkSubpassDependency dependency = {};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		VkRenderPassCreateInfo render_pass_info = {};
		render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		render_pass_info.attachmentCount = 1; // static_cast<uint32_t>(attachments.size());
		render_pass_info.pAttachments = attachments.data();
		render_pass_info.subpassCount = 1;
		render_pass_info.pSubpasses = &subpass_description;
		render_pass_info.dependencyCount = 1;
		render_pass_info.pDependencies = &dependency;

		if (vkCreateRenderPass(m_device->GetVkDevice(), &render_pass_info, nullptr, &m_render_pass) != VK_SUCCESS)
			KB_CORE_ASSERT(false, "Vulkan failed to create render pass!");

		CreateFramebuffer();

	}

	void VulkanSwapChain::OnResize(uint32_t width, uint32_t height)
	{
		auto device = m_device->GetVkDevice();

		// wait to synchronize
		vkDeviceWaitIdle(device);

		Create(&width, &height, m_vsync);

		//for (auto& framebuffer : m_framebuffers)
		//	vkDestroyFramebuffer(device, framebuffer, nullptr);

		CreateFramebuffer();

		vkDeviceWaitIdle(device);
	}

	void VulkanSwapChain::BeginFrame()
	{
		// Make sure the frame we're requesting has finished rendering
		uint32_t frames_in_flight = render::get_frames_in_flights();
		if (vkWaitForFences(m_device->GetVkDevice(), 1, &m_wait_fences[(m_current_buffer_index + 2) % frames_in_flight], VK_TRUE, UINT64_MAX) != VK_SUCCESS)
			KB_CORE_ASSERT(false, "Vulkan failed to wait for fences");

		// execute resource release queue

		auto& queue = render::get_render_resource_release_queue(m_current_buffer_index);
		queue.Execute();

		if (vkWaitForFences(m_device->GetVkDevice(), 1, &m_wait_fences[m_current_buffer_index], VK_TRUE, UINT64_MAX) != VK_SUCCESS)
			KB_CORE_ASSERT(false, "Vulkan failed to wait for fences");

		if (vkResetCommandPool(m_device->GetVkDevice(), m_command_pool, 0))
			KB_CORE_ASSERT(false, "Vulkan failed to reset command pool!");

		if (AcquireNextImage(m_semaphores.present_complete, &m_current_image_index) != VK_SUCCESS)
			KB_CORE_ERROR("VulkanSwapChain BeginFrame failed to acquire next image!");
	}

	void VulkanSwapChain::Present()
	{
		constexpr uint64_t DEFAULT_FENCE_TIMEOUT = 100000000000u;

		VkPipelineStageFlags wait_stage_mask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

		VkSubmitInfo submit_info{};
		submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submit_info.pWaitDstStageMask = &wait_stage_mask;
		submit_info.waitSemaphoreCount = 1;
		submit_info.pWaitSemaphores = &m_semaphores.present_complete;
		submit_info.signalSemaphoreCount = 1;
		submit_info.pSignalSemaphores = &m_semaphores.render_complete;
		submit_info.pCommandBuffers = &m_command_buffers[m_current_buffer_index];
		submit_info.commandBufferCount = 1;

		if (vkResetFences(m_device->GetVkDevice(), 1, &m_wait_fences[m_current_buffer_index]) != VK_SUCCESS)
			KB_CORE_ASSERT(false, "Vulkan failed to reset fence!");

		if (vkQueueSubmit(m_device->GetGraphicsQueue(), 1, &submit_info, m_wait_fences[m_current_buffer_index]) != VK_SUCCESS)
			KB_CORE_ASSERT(false, "Vulkan failed to submit!");

		// present buffer to the swap chain
		VkResult result = QueuePresent(m_device->GetGraphicsQueue(), m_current_image_index, m_semaphores.render_complete);

		if (result != VK_SUCCESS || result == VK_SUBOPTIMAL_KHR)
		{
			if (result == VK_ERROR_OUT_OF_DATE_KHR)
			{
				OnResize(m_width, m_height);
				return;
			}
		}

		m_current_buffer_index = (m_current_image_index + 1) % render::get_frames_in_flights();
		if (vkWaitForFences(m_device->GetVkDevice(), 1, &m_wait_fences[m_current_buffer_index], VK_TRUE, UINT64_MAX) != VK_SUCCESS)
			KB_CORE_ASSERT(false, "Vulkan failed to wait for fences!");
	}

	void VulkanSwapChain::Destroy()
	{
		VkDevice device = m_device->GetVkDevice();

		if (m_swapchain)
		{
			for (uint32_t i = 0; i < m_image_count; ++i)
				vkDestroyImageView(device, m_buffers[i].view, nullptr);
		}

		if (m_surface)
		{
			vkDestroySwapchainKHR(device, m_swapchain, nullptr);
			vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
		}

		if (m_command_pool)
			vkDestroyCommandPool(device, m_command_pool, nullptr);

		if (m_render_pass)
			vkDestroyRenderPass(device, m_render_pass, nullptr);

		for (auto& framebuffer : m_framebuffers)
			vkDestroyFramebuffer(device, framebuffer, nullptr);

		if (m_semaphores.present_complete)
			vkDestroySemaphore(device, m_semaphores.present_complete, nullptr);

		if (m_semaphores.render_complete)
			vkDestroySemaphore(device, m_semaphores.render_complete, nullptr);

		for (auto& fence : m_wait_fences)
			vkDestroyFence(device, fence, nullptr);

		m_swapchain = nullptr;
		m_surface = nullptr;
	}

	VkResult VulkanSwapChain::AcquireNextImage(VkSemaphore present_complete_sem, uint32_t* image_index)
	{
		// why no fence?
		return vkAcquireNextImageKHR(m_device->GetVkDevice(), m_swapchain, UINT64_MAX, present_complete_sem, (VkFence)nullptr, image_index);
	}

	VkResult VulkanSwapChain::QueuePresent(VkQueue queue, uint32_t image_index, VkSemaphore wait_sem /*= VK_NULL_HANDLE*/)
	{
		VkPresentInfoKHR present_info{};
		present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		present_info.pNext = nullptr;
		present_info.swapchainCount = 1;
		present_info.pSwapchains = &m_swapchain;
		present_info.pImageIndices = &image_index;

		if (wait_sem != VK_NULL_HANDLE)
		{
			present_info.pWaitSemaphores = &wait_sem;
			present_info.waitSemaphoreCount = 1;
		}

		return vkQueuePresentKHR(queue, &present_info);
	}

	void VulkanSwapChain::FindImageFormatAndColorSpace()
	{
		VkPhysicalDevice physical_device = m_device->GetVkPhysicalDevice();

		uint32_t format_count = 0;
		if (vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, m_surface, &format_count, nullptr) != VK_SUCCESS)
			KB_CORE_ASSERT(false, "Vulkan could not find image format count!");

		std::vector<VkSurfaceFormatKHR> formats(format_count);
		if (vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, m_surface, &format_count, formats.data()) != VK_SUCCESS)
			KB_CORE_ASSERT(false, "Vulkan could not find image formats!");

		if ((format_count == 1) && (formats[0].format == VK_FORMAT_UNDEFINED))
		{
			m_color_format = VK_FORMAT_B8G8R8_UNORM;
			m_color_space = formats[0].colorSpace;
		}
		else
		{
			bool found = false;
			for (auto&& format : formats)
			{
				if (format.format == VK_FORMAT_B8G8R8_UNORM)
				{
					m_color_format = VK_FORMAT_B8G8R8_UNORM;
					m_color_space = format.colorSpace;
					found = true;
					break;
				}
			}

			if (!found)
			{
				m_color_format = formats[0].format;
				m_color_space = formats[0].colorSpace;
			}
		}
	}

	void VulkanSwapChain::CreateFramebuffer()
	{
		for (auto& framebuffer : m_framebuffers)
			if (framebuffer)
				vkDestroyFramebuffer(m_device->GetVkDevice(), framebuffer, nullptr);

		VkImageView image_view_attachments[2];

		// Depth stencil not needed?
		//image_view_attachments[1] = m_depth_stencil.image_view;

		VkFramebufferCreateInfo frame_buffer_create_info{};
		frame_buffer_create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		frame_buffer_create_info.pNext = nullptr;
		frame_buffer_create_info.renderPass = m_render_pass;
		frame_buffer_create_info.attachmentCount = 1;
		frame_buffer_create_info.pAttachments = image_view_attachments;
		frame_buffer_create_info.width = m_width;
		frame_buffer_create_info.height = m_height;
		frame_buffer_create_info.layers = 1;

		m_framebuffers.resize(m_image_count);
		for (uint32_t i = 0; i < m_image_count; ++i)
		{
			image_view_attachments[0] = m_buffers[i].view;
			if (vkCreateFramebuffer(m_device->GetVkDevice(), &frame_buffer_create_info, nullptr, &m_framebuffers[i]) != VK_SUCCESS)
				KB_CORE_ASSERT(false, "Vulkan failed to create framebuffer!");
		}
	}

	void VulkanSwapChain::CreateDepthStencil()
	{
		VkDevice device = m_device->GetVkDevice();
		VkFormat depth_format = m_device->GetPhysicalDevice()->GetDepthFormat();

		VkImageCreateInfo image_create_info{};
		image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		image_create_info.imageType = VK_IMAGE_TYPE_2D;
		image_create_info.format = depth_format;
		image_create_info.extent = { m_width, m_height + 1 };
		image_create_info.mipLevels = 1;
		image_create_info.arrayLayers = 1;
		image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
		image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
		image_create_info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
		image_create_info.pQueueFamilyIndices = nullptr;

		VulkanAllocator allocator{ "Swapchain Depth Stencil" };
		m_depth_stencil.memory_allocation = allocator.AllocateImage(image_create_info, VMA_MEMORY_USAGE_GPU_ONLY, m_depth_stencil.image);

		VkImageViewCreateInfo image_view_create_info;
		image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
		image_view_create_info.image = m_depth_stencil.image;
		image_view_create_info.format = depth_format;
		image_view_create_info.subresourceRange.baseMipLevel = 0;
		image_view_create_info.subresourceRange.levelCount = 1;
		image_view_create_info.subresourceRange.baseArrayLayer = 0;
		image_view_create_info.subresourceRange.layerCount = 1;
		image_view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

		if (depth_format >= VK_FORMAT_D16_UNORM_S8_UINT)
			image_view_create_info.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;

		if (vkCreateImageView(device, &image_view_create_info, nullptr, &m_depth_stencil.image_view) != VK_SUCCESS)
			KB_CORE_ASSERT(false, "Vulkan failed to create depth stencil image view!");
	}
}
