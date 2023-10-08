#include "kablunkpch.h"

#include "Platform/Vulkan/VulkanFramebuffer.h"
#include "Platform/Vulkan/VulkanSwapChain.h"
#include "Platform/Vulkan/VulkanContext.h"
#include "Platform/Vulkan/VulkanAllocator.h"
#include "Platform/Vulkan/VulkanImage.h"
#include "Platform/Vulkan/VulkanRenderer.h"

#include "Kablunk/Core/Application.h"

#include "Kablunk/Renderer/Image.h"

namespace kb
{

	VulkanFramebuffer::VulkanFramebuffer(const FramebufferSpecification& spec)
		: m_specification{ spec }
	{
		if (m_specification.width == 0 || m_specification.height == 0)
		{
			m_width = Application::Get().GetWindow().GetWidth();
			m_height = Application::Get().GetWindow().GetHeight();
		}
		else
		{
			m_width = m_specification.width;
			m_height = m_specification.height;
		}

		// Create images
		uint32_t attachment_index = 0;
		if (!m_specification.existing_framebuffer)
		{
			for (auto& attachment_specification : m_specification.Attachments.Attachments)
			{
				if (m_specification.existing_image && m_specification.existing_image->GetSpecification().deinterleaved)
				{
					KB_CORE_ASSERT(!Utils::IsDepthFormat(attachment_specification.format), "Only supported for color attachments");
					m_attachment_images.emplace_back(m_specification.existing_image);
				}
				else if (m_specification.existing_images.find(attachment_index) != m_specification.existing_images.end())
				{
					if (!Utils::IsDepthFormat(attachment_specification.format))
						m_attachment_images.emplace_back(); // This will be set later
				}
				else if (Utils::IsDepthFormat(attachment_specification.format))
				{
					ImageSpecification spec;
					spec.format = attachment_specification.format;
					spec.usage = ImageUsage::Attachment;
					spec.width = static_cast<uint32_t>(std::ceil(static_cast<float>(m_width) * m_specification.scale));
					spec.height = static_cast<uint32_t>(std::ceil(static_cast<float>(m_height) * m_specification.scale));
					spec.debug_name = fmt::format("{0}-DepthAttachment{1}", m_specification.debug_name.empty() ? "Unnamed FB" : m_specification.debug_name, attachment_index);
					m_depth_attachment_image = Image2D::Create(spec);
				}
				else
				{
					ImageSpecification spec;
					spec.format = attachment_specification.format;
					spec.usage = ImageUsage::Attachment;
					spec.width = static_cast<uint32_t>(std::ceil(static_cast<float>(m_width) * m_specification.scale));
					spec.height = static_cast<uint32_t>(std::ceil(static_cast<float>(m_height) * m_specification.scale));
					spec.debug_name = fmt::format("{0}-ColorAttachment{1}", m_specification.debug_name.empty() ? "Unnamed FB" : m_specification.debug_name, attachment_index);
					m_attachment_images.emplace_back(Image2D::Create(spec));
				}
				attachment_index++;
			}
		}

		KB_CORE_ASSERT(m_specification.Attachments.Attachments.size(), "no images were created!");
		Resize(m_width, m_height, true);
	}

	VulkanFramebuffer::~VulkanFramebuffer()
	{
		if (!m_framebuffer)
			return;

		KB_CORE_INFO("Destroying VulkanFramebuffer '{0}'", m_specification.debug_name);
		VkFramebuffer vk_framebuffer = m_framebuffer;
		VkRenderPass vk_render_pass = m_render_pass;
		render::submit_resource_free([vk_framebuffer, vk_render_pass]()
			{
				const auto device = VulkanContext::Get()->GetDevice()->GetVkDevice();
				vkDestroyFramebuffer(device, vk_framebuffer, nullptr);
				vkDestroyRenderPass(device, vk_render_pass, nullptr);
			});

		// Only destroy images we own
		if (!m_specification.existing_framebuffer)
		{
			uint32_t attachment_index = 0;
			for (ref<VulkanImage2D> image : m_attachment_images)
			{
				if (m_specification.existing_images.find(attachment_index) != m_specification.existing_images.end())
					continue;

				// Only destroy deinterleaved image once and prevent clearing layer views on second framebuffer invalidation
				if (!image->GetSpecification().deinterleaved || attachment_index == 0 && !image->GetLayerImageView(0))
					image->Release();

				attachment_index++;
			}

			if (m_depth_attachment_image)
			{
				// Do we own the depth image?
				if (m_specification.existing_images.find(static_cast<uint32_t>(m_specification.Attachments.Attachments.size()) - 1) == m_specification.existing_images.end())
					m_depth_attachment_image->Release();
			}

		}
	}

	void VulkanFramebuffer::Resize(uint32_t width, uint32_t height, bool force_recreate /*= false*/)
	{
		if (!force_recreate && (m_width == width && m_height == height))
			return;

		ref<VulkanFramebuffer> instance = this;
		render::submit([instance, width, height]() mutable
			{
				instance->m_width = static_cast<uint32_t>(std::ceil(static_cast<float>(width) * instance->m_specification.scale));
				instance->m_height = static_cast<uint32_t>(std::ceil(static_cast<float>(height) * instance->m_specification.scale));
				if (!instance->m_specification.swap_chain_target)
					instance->RT_Invalidate();
				else
				{
					VulkanSwapChain& swapChain = VulkanContext::Get()->GetSwapchain();
					instance->m_render_pass = swapChain.GetRenderPass();

					instance->m_clear_values.clear();
                    const auto& clear_color = instance->m_specification.clear_color;
					instance->m_clear_values.emplace_back().color = { clear_color.r, clear_color.g, clear_color.b, clear_color.a };
				}
			});

		for (auto& callback : m_resize_callbacks)
			callback(this);
	}

	void VulkanFramebuffer::AddResizeCallback(const std::function<void(ref<Framebuffer>)>& func)
	{
		m_resize_callbacks.push_back(func);
	}

	int VulkanFramebuffer::ReadPixel(uint32_t attachment_index, int x, int y)
	{
		KB_CORE_ASSERT(false, "Not implemented in Vulkan!");
		return 0;
#if 0
		KB_CORE_ASSERT(attachment_index < m_attachment_images.size(), "out of bounds!");

		IntrusiveRef<Image2D> attachment = m_attachment_images[attachment_index];

		Buffer& attachment_buffer = attachment->GetBuffer();
		if (!attachment_buffer)
			return -1;

		return attachment_buffer[y * attachment->GetWidth() + x];
#endif
	}

	void VulkanFramebuffer::ClearAttachment(uint32_t attachment_index, int value)
	{
		// attachments in swapchain are automatically cleared
	}

	void VulkanFramebuffer::Invalidate()
	{
		ref<VulkanFramebuffer> instance = this;
		render::submit([instance]() mutable
			{
				instance->RT_Invalidate();
			});
	}

	void VulkanFramebuffer::RT_Invalidate()
	{
		VkDevice device = VulkanContext::Get()->GetDevice()->GetVkDevice();

		if (m_framebuffer)
		{
			VkFramebuffer vk_framebuffer = m_framebuffer;
			render::submit_resource_free([vk_framebuffer]()
				{
					const auto device = VulkanContext::Get()->GetDevice()->GetVkDevice();
					vkDestroyFramebuffer(device, vk_framebuffer, nullptr);
				});

			// Don't free the images if we don't own them
			if (!m_specification.existing_framebuffer)
			{
				uint32_t attachment_index = 0;
				for (ref<VulkanImage2D> image : m_attachment_images)
				{
					if (m_specification.existing_images.find(attachment_index) != m_specification.existing_images.end())
						continue;

					// Only destroy deinterleaved image once and prevent clearing layer views on second framebuffer invalidation
					if (!image->GetSpecification().deinterleaved || attachment_index == 0 && !image->GetLayerImageView(0))
						image->Release();

					attachment_index++;
				}

				if (m_depth_attachment_image)
				{
					// Do we own the depth image?
					if (m_specification.existing_images.find(static_cast<uint32_t>(m_specification.Attachments.Attachments.size()) - 1) == m_specification.existing_images.end())
						m_depth_attachment_image->Release();
				}

			}
		}

		VulkanAllocator allocator{ "Framebuffer" };

		std::vector<VkAttachmentDescription> attachment_descriptions;

		std::vector<VkAttachmentReference> color_attachment_references;
		VkAttachmentReference depth_attachment_reference;

		m_clear_values.resize(m_specification.Attachments.Attachments.size());

		bool create_images = m_attachment_images.empty();

		if (m_specification.existing_framebuffer)
			m_attachment_images.clear();

		uint32_t attachment_index = 0;
		for (const auto& attachment_spec : m_specification.Attachments.Attachments)
		{
			if (Utils::IsDepthFormat(attachment_spec.format))
			{
				if (m_specification.existing_image)
					m_depth_attachment_image = m_specification.existing_image;
				else if (m_specification.existing_framebuffer)
				{
					ref<VulkanFramebuffer> existing_framebuffer = m_specification.existing_framebuffer.As<VulkanFramebuffer>();
					m_depth_attachment_image = existing_framebuffer->GetDepthImage();
				}
				else if (m_specification.existing_images.find(attachment_index) != m_specification.existing_images.end())
				{
					ref<Image2D> existing_image = m_specification.existing_images.at(attachment_index);
					KB_CORE_ASSERT(Utils::IsDepthFormat(existing_image->GetSpecification().format), "Trying to attach non-depth image as depth attachment");
					m_depth_attachment_image = existing_image;
				}
				else
				{
					ref<VulkanImage2D> depth_attachment_image = m_depth_attachment_image.As<VulkanImage2D>();
					auto& spec = depth_attachment_image->GetSpecification();
					spec.width = m_width;
					spec.height = m_height;
					depth_attachment_image->RT_Invalidate(); // Create immediately
				}

				VkAttachmentDescription& attachment_description = attachment_descriptions.emplace_back();
				attachment_description.flags = 0;
				attachment_description.format = Utils::VulkanImageFormat(attachment_spec.format);
				attachment_description.samples = VK_SAMPLE_COUNT_1_BIT;
				attachment_description.loadOp = m_specification.clear_on_load ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
				attachment_description.storeOp = VK_ATTACHMENT_STORE_OP_STORE; // TODO: if sampling, needs to be store (otherwise DONT_CARE is fine)
				attachment_description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
				attachment_description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
				attachment_description.initialLayout = m_specification.clear_on_load ? VK_IMAGE_LAYOUT_UNDEFINED : VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
				if (attachment_spec.format == ImageFormat::DEPTH24STENCIL8 || true) // Separate layouts requires a "separate layouts" flag to be enabled
				{
					attachment_description.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL; // TODO: if not sampling
					attachment_description.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL; // TODO: if sampling
					depth_attachment_reference = { attachment_index, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };
				}
				else
				{
					attachment_description.finalLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL; // TODO: if not sampling
					attachment_description.finalLayout = VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL; // TODO: if sampling
					depth_attachment_reference = { attachment_index, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL };
				}
				m_clear_values[attachment_index].depthStencil = { 1.0f, 0 };
			}
			else
			{
				//HZ_CORE_ASSERT(!m_Specification.ExistingImage, "Not supported for color attachments");

				ref<VulkanImage2D> color_attachment;
				if (m_specification.existing_framebuffer)
				{
					ref<VulkanFramebuffer> existing_framebuffer = m_specification.existing_framebuffer.As<VulkanFramebuffer>();
					ref<Image2D> existingImage = existing_framebuffer->GetImage(attachment_index);
					color_attachment = m_attachment_images.emplace_back(existingImage).As<VulkanImage2D>();
				}
				else if (m_specification.existing_images.find(attachment_index) != m_specification.existing_images.end())
				{
					ref<Image2D> existing_image = m_specification.existing_images[attachment_index];
					KB_CORE_ASSERT(!Utils::IsDepthFormat(existing_image->GetSpecification().format), "Trying to attach depth image as color attachment");
					color_attachment = existing_image.As<VulkanImage2D>();
					m_attachment_images[attachment_index] = existing_image;
				}
				else
				{
					if (create_images)
					{
						ImageSpecification spec;
						spec.format = attachment_spec.format;
						spec.usage = ImageUsage::Attachment;
						spec.width = m_width;
						spec.height = m_height;
						color_attachment = m_attachment_images.emplace_back(Image2D::Create(spec)).As<VulkanImage2D>();
					}
					else
					{
						ref<Image2D> image = m_attachment_images[attachment_index];
						ImageSpecification& spec = image->GetSpecification();
						spec.width = m_width;
						spec.height = m_height;
						color_attachment = image.As<VulkanImage2D>();
						if (!color_attachment->GetSpecification().deinterleaved)
							color_attachment->RT_Invalidate(); // Create immediately
						else if (attachment_index == 0 && m_specification.existing_image_layers[0] == 0)// Only invalidate the first layer from only the first framebuffer
						{
							color_attachment->RT_Invalidate(); // Create immediately
							color_attachment->RT_CreatePerSpecificLayerImageViews(m_specification.existing_image_layers);
						}
						else if (attachment_index == 0)
						{
							color_attachment->RT_CreatePerSpecificLayerImageViews(m_specification.existing_image_layers);
						}

						if (image.As<VulkanImage2D>()->GetDescriptor().imageLayout == VK_IMAGE_LAYOUT_UNDEFINED)
							color_attachment->RT_Invalidate();
					}
				}

				VkAttachmentDescription& attachment_description = attachment_descriptions.emplace_back();
				attachment_description.flags = 0;
				attachment_description.format = Utils::VulkanImageFormat(attachment_spec.format);
				attachment_description.samples = VK_SAMPLE_COUNT_1_BIT;
				attachment_description.loadOp = m_specification.clear_on_load ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
				attachment_description.storeOp = VK_ATTACHMENT_STORE_OP_STORE; // TODO: if sampling, needs to be store (otherwise DONT_CARE is fine)
				attachment_description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
				attachment_description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
				attachment_description.initialLayout = m_specification.clear_on_load ? VK_IMAGE_LAYOUT_UNDEFINED : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				attachment_description.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

				const auto& clear_color = m_specification.clear_color;
				m_clear_values[attachment_index].color = { {clear_color.r, clear_color.g, clear_color.b, clear_color.a} };
				color_attachment_references.emplace_back(VkAttachmentReference{ attachment_index, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });
			}

			attachment_index++;
		}

		VkSubpassDescription subpass_description = {};
		subpass_description.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass_description.colorAttachmentCount = static_cast<uint32_t>(color_attachment_references.size());
		subpass_description.pColorAttachments = color_attachment_references.data();
		if (m_depth_attachment_image)
			subpass_description.pDepthStencilAttachment = &depth_attachment_reference;

		// TODO: do we need these?
		// Use subpass dependencies for layout transitions
		std::vector<VkSubpassDependency> dependencies;

		if (m_attachment_images.size())
		{
			{
				VkSubpassDependency& depedency = dependencies.emplace_back();
				depedency.srcSubpass = VK_SUBPASS_EXTERNAL;
				depedency.dstSubpass = 0;
				depedency.srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
				depedency.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
				depedency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
				depedency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
				depedency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
			}

			{
				VkSubpassDependency& depedency = dependencies.emplace_back();
				depedency.srcSubpass = 0;
				depedency.dstSubpass = VK_SUBPASS_EXTERNAL;
				depedency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
				depedency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
				depedency.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
				depedency.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
				depedency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
			}
		}

		if (m_depth_attachment_image)
		{
			{
				VkSubpassDependency& depedency = dependencies.emplace_back();
				depedency.srcSubpass = VK_SUBPASS_EXTERNAL;
				depedency.dstSubpass = 0;
				depedency.srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
				depedency.dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
				depedency.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
				depedency.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
				depedency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
			}

			{
				VkSubpassDependency& depedency = dependencies.emplace_back();
				depedency.srcSubpass = 0;
				depedency.dstSubpass = VK_SUBPASS_EXTERNAL;
				depedency.srcStageMask = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
				depedency.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
				depedency.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
				depedency.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
				depedency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
			}
		}

		// Create the actual renderpass
		VkRenderPassCreateInfo render_pass_info = {};
		render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		render_pass_info.attachmentCount = static_cast<uint32_t>(attachment_descriptions.size());
		render_pass_info.pAttachments = attachment_descriptions.data();
		render_pass_info.subpassCount = 1;
		render_pass_info.pSubpasses = &subpass_description;
		render_pass_info.dependencyCount = static_cast<uint32_t>(dependencies.size());
		render_pass_info.pDependencies = dependencies.data();

		if (vkCreateRenderPass(device, &render_pass_info, nullptr, &m_render_pass) != VK_SUCCESS)
			KB_CORE_ASSERT(false, "Vulkan failed to create render pass");

		std::vector<VkImageView> attachments(m_attachment_images.size());
		for (uint32_t i = 0; i < m_attachment_images.size(); i++)
		{
			ref<VulkanImage2D> image = m_attachment_images[i].As<VulkanImage2D>();
			if (image->GetSpecification().deinterleaved)
			{
				attachments[i] = image->GetLayerImageView(m_specification.existing_image_layers[i]);
				KB_CORE_ASSERT(attachments[i], "Attachment invalid!");
			}
			else
			{
				attachments[i] = image->GetImageInfo().image_view;
				KB_CORE_ASSERT(attachments[i], "Attachment invalid!");
			}
		}

		if (m_depth_attachment_image)
		{
			ref<VulkanImage2D> image = m_depth_attachment_image.As<VulkanImage2D>();
			if (m_specification.existing_image)
			{
				KB_CORE_ASSERT(m_specification.existing_image_layers.size() == 1, "Depth attachments do not support deinterleaving");
				attachments.emplace_back(image->GetLayerImageView(m_specification.existing_image_layers[0]));
				KB_CORE_ASSERT(attachments.back(), "error");
			}
			else
			{
				attachments.emplace_back(image->GetImageInfo().image_view);
				KB_CORE_ASSERT(attachments.back(), "error");
			}
		}

		VkFramebufferCreateInfo frame_buffer_create_info = {};
		frame_buffer_create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		frame_buffer_create_info.renderPass = m_render_pass;
		frame_buffer_create_info.attachmentCount = static_cast<uint32_t>(attachments.size());
		frame_buffer_create_info.pAttachments = attachments.data();
		frame_buffer_create_info.width = m_width;
		frame_buffer_create_info.height = m_height;
		frame_buffer_create_info.layers = 1;

		if (vkCreateFramebuffer(device, &frame_buffer_create_info, nullptr, &m_framebuffer) != VK_SUCCESS)
			KB_CORE_ASSERT(false, "Vulkan failed to create frame buffer!");
	}

}
