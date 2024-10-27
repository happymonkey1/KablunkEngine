#include "kablunkpch.h"

#include "kablunk/renderer/backend/vulkan/vulkan_frame_buffer.h"
#include "kablunk/renderer/backend/vulkan/vulkan_swap_chain.h"
#include "kablunk/renderer/backend/vulkan/vulkan_context.h"
#include "kablunk/renderer/backend/vulkan/vulkan_allocator.h"
#include "kablunk/renderer/backend/vulkan/VulkanImage.h"
#include "kablunk/renderer/backend/vulkan/VulkanRenderer.h"

#include "Kablunk/Core/Application.h"

#include "Kablunk/Renderer/backend/image.h"
#include "Kablunk/renderer/backend/vulkan/vulkan_utils.h"

namespace kb::render::backend::vk
{ // start namespace kb::render

namespace utils
{ // start namespace ::utils
inline auto get_vk_attachment_load_op(
    const frame_buffer_specification& p_specification,
    const frame_buffer_texture_specification& p_texture_specification
) noexcept -> VkAttachmentLoadOp
{
    switch (p_texture_specification.m_load_op)
    {
    case attachment_load_op_t::inherit:
    {
        // #TODO should have separate depth and color clears...
        if (backend::util::IsDepthFormat(p_texture_specification.format))
            return p_specification.m_clear_depth_on_load ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;

        return p_specification.m_clear_color_on_load ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
    }
    case attachment_load_op_t::clear:
        return VK_ATTACHMENT_LOAD_OP_CLEAR;
    case attachment_load_op_t::load:
        return VK_ATTACHMENT_LOAD_OP_LOAD;
    default:
        KB_CORE_ASSERT(false, "[vulkan_frame_buffer]: Unhandled attachment_load_op_t!");
        return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    }
}
} // end namespace ::utils

vulkan_frame_buffer::vulkan_frame_buffer(frame_buffer_specification spec)
	: m_specification{std::move(spec)}
{
	if (m_specification.m_width == 0 || m_specification.m_height == 0)
	{
		m_width = Application::Get().GetWindow().GetWidth();
		m_height = Application::Get().GetWindow().GetHeight();
	}
	else
	{
		m_width = m_specification.m_width;
		m_height = m_specification.m_height;
	}

	// Create images
    if (!m_specification.m_existing_frame_buffer)
    {
        uint32_t attachment_index = 0;
        for (auto& attachment_specification : m_specification.m_attachments.Attachments)
		{
			if (m_specification.m_existing_image)
			{
                if (backend::util::IsDepthFormat(attachment_specification.format))
                    m_depth_attachment_image = m_specification.m_existing_image;
                else
				    m_attachment_images.emplace_back(m_specification.m_existing_image);
			}
			else if (m_specification.m_existing_images.contains(attachment_index))
			{
				if (backend::util::IsDepthFormat(attachment_specification.format))
				{
                    m_depth_attachment_image = m_specification.m_existing_images.at(attachment_index);
				}
                else
                {
                    m_attachment_images.emplace_back(); // This will be set later
                }
			}
			else if (backend::util::IsDepthFormat(attachment_specification.format))
			{
				image_specification_t image_create_spec;
                image_create_spec.format = attachment_specification.format;
                image_create_spec.usage = image_usage_t::Attachment;
                image_create_spec.width = static_cast<uint32_t>(std::ceil(static_cast<float>(m_width) * m_specification.m_scale));
                image_create_spec.height = static_cast<uint32_t>(std::ceil(static_cast<float>(m_height) * m_specification.m_scale));
                image_create_spec.m_transfer = m_specification.m_transfer;
                image_create_spec.debug_name = fmt::format("{0}-DepthAttachment{1}", m_specification.m_debug_name.empty() ? "Unnamed FB" : m_specification.m_debug_name, attachment_index);
				m_depth_attachment_image = image_2d::create(image_create_spec);
			}
			else
			{
				image_specification_t image_create_spec;
                image_create_spec.format = attachment_specification.format;
                image_create_spec.usage = image_usage_t::Attachment;
                image_create_spec.width = static_cast<uint32_t>(std::ceil(static_cast<float>(m_width) * m_specification.m_scale));
                image_create_spec.height = static_cast<uint32_t>(std::ceil(static_cast<float>(m_height) * m_specification.m_scale));
                image_create_spec.m_transfer = m_specification.m_transfer;
                image_create_spec.debug_name = fmt::format("{0}-ColorAttachment{1}", m_specification.m_debug_name.empty() ? "Unnamed FB" : m_specification.m_debug_name, attachment_index);
				m_attachment_images.emplace_back(image_2d::create(image_create_spec));
			}
			attachment_index++;
		}
	}

	KB_CORE_ASSERT(!m_specification.m_attachments.Attachments.empty(), "no images were created!");
    vulkan_frame_buffer::resize(m_width, m_height, true);
}

vulkan_frame_buffer::~vulkan_frame_buffer()
{
	if (!m_framebuffer)
		return;

	KB_CORE_INFO("Destroying VulkanFramebuffer '{0}'", m_specification.m_debug_name);
	VkFramebuffer vk_frame_buffer = m_framebuffer;
	VkRenderPass vk_render_pass = m_vk_render_pass;
	render::submit_resource_free([vk_frame_buffer, vk_render_pass]()
		{
			const auto device = vulkan_context::get()->get_device()->get_vk_device();
			vkDestroyFramebuffer(device, vk_frame_buffer, nullptr);
			vkDestroyRenderPass(device, vk_render_pass, nullptr);
		});

	// Only destroy images we own
	if (!m_specification.m_existing_frame_buffer)
	{
		uint32_t attachment_index = 0;
		for (arc<image_2d>& image : m_attachment_images)
		{
			if (m_specification.m_existing_images.find(attachment_index) != m_specification.m_existing_images.end())
				continue;

            // Only destroy deinterleaved image once and prevent clearing layer views on second framebuffer invalidation
			if (arc<vulkan_image_2d> vk_image = image.As<vulkan_image_2d>(); !vk_image->get_specification().deinterleaved ||
                attachment_index == 0 && !vk_image->GetLayerImageView(0))
			{
                vk_image->release();
			}

			attachment_index++;
		}

		if (m_depth_attachment_image)
		{
			// Do we own the depth image?
			if (!m_specification.m_existing_images.contains(static_cast<uint32_t>(m_specification.m_attachments.Attachments.size()) - 1))
				m_depth_attachment_image->release();
		}
	}
}

void vulkan_frame_buffer::resize(uint32_t width, uint32_t height, bool force_recreate /*= false*/)
{
	if (!force_recreate && (m_width == width && m_height == height))
		return;

    arc instance{ this };
	render::submit([instance, width, height]() mutable
		{
			instance->m_width = static_cast<uint32_t>(
                std::ceil(static_cast<float>(width) * instance->m_specification.m_scale)
            );
			instance->m_height = static_cast<uint32_t>(
                std::ceil(static_cast<float>(height) * instance->m_specification.m_scale)
            );
			if (!instance->m_specification.m_swap_chain_target)
				instance->rt_invalidate();
			else
			{
				instance->m_vk_render_pass = vulkan_context::get()->get_vulkan_swap_chain()->get_vk_render_pass();

				instance->m_clear_values.clear();
                const auto& clear_color = instance->m_specification.m_clear_color;
				instance->m_clear_values.emplace_back().color = { clear_color.r, clear_color.g, clear_color.b, clear_color.a };
			}
		});

	for (auto& callback : m_resize_callbacks)
		callback(arc<frame_buffer>{ this });
}

void vulkan_frame_buffer::add_resize_callback(const std::function<void(arc<frame_buffer>)>& func)
{
	m_resize_callbacks.push_back(func);
}

int vulkan_frame_buffer::read_pixel(uint32_t attachment_index, int x, int y)
{
	KB_CORE_ASSERT(false, "Not implemented in Vulkan!");
	return 0;
#if 0
	KB_CORE_ASSERT(attachment_index < m_attachment_images.size(), "out of bounds!");

	IntrusiveRef<Image2D> attachment = m_attachment_images[attachment_index];

	owning_buffer& attachment_buffer = attachment->GetBuffer();
	if (!attachment_buffer)
		return -1;

	return attachment_buffer[y * attachment->GetWidth() + x];
#endif
}

void vulkan_frame_buffer::clear_attachment(uint32_t attachment_index, int value)
{
	// attachments in swapchain are automatically cleared
}

void vulkan_frame_buffer::invalidate()
{
    arc<vulkan_frame_buffer> instance{ this };
	render::submit([instance]() mutable
		{
			instance->rt_invalidate();
		});
}

void vulkan_frame_buffer::rt_invalidate()
{
    log::core::trace(
        log::logger_tag_t::framebuffer,
        "[vulkan_frame_buffer]: RT_Invalidate for frame buffer {}",
        static_cast<const void*>(&m_framebuffer)
    );
	VkDevice device = vulkan_context::get()->get_device()->get_vk_device();

	if (m_framebuffer)
	{
		VkFramebuffer vk_frame_buffer = m_framebuffer;
		render::submit_resource_free([vk_frame_buffer]()
			{
				const auto device = vulkan_context::get()->get_device()->get_vk_device();
				vkDestroyFramebuffer(device, vk_frame_buffer, nullptr);
			});

		// Don't free the images if we don't own them
		if (!m_specification.m_existing_frame_buffer)
		{
			uint32_t attachment_index = 0;
			for (const arc<image_2d>& image : m_attachment_images)
			{
				if (m_specification.m_existing_images.contains(attachment_index))
					continue;

                // Only destroy deinterleaved image once and prevent clearing layer views on second framebuffer invalidation
				if (arc vk_image = image.As<vulkan_image_2d>(); !vk_image->get_specification().deinterleaved || attachment_index == 0 && !vk_image->GetLayerImageView(0))
				{
                    vk_image->release();
				}

				attachment_index++;
			}

			if (m_depth_attachment_image)
			{
				// Do we own the depth image?
				if (!m_specification.m_existing_images.contains(static_cast<uint32_t>(m_specification.m_attachments.Attachments.size()) - 1))
					m_depth_attachment_image->release();
			}
		}
	}

	vulkan_allocator allocator{ "Framebuffer" };

	std::vector<VkAttachmentDescription> attachment_descriptions;

	std::vector<VkAttachmentReference> color_attachment_references;
	VkAttachmentReference depth_attachment_reference;

	m_clear_values.resize(m_specification.m_attachments.Attachments.size());

	bool create_images = m_attachment_images.empty();

	if (m_specification.m_existing_frame_buffer)
		m_attachment_images.clear();

	uint32_t attachment_index = 0;
	for (const auto& attachment_spec : m_specification.m_attachments.Attachments)
	{
		if (backend::util::IsDepthFormat(attachment_spec.format))
		{
			if (m_specification.m_existing_image)
				m_depth_attachment_image = m_specification.m_existing_image;
			else if (m_specification.m_existing_frame_buffer)
			{
				arc<vulkan_frame_buffer> existing_frame_buffer = m_specification.m_existing_frame_buffer.As<vulkan_frame_buffer>();
				m_depth_attachment_image = existing_frame_buffer->get_depth_image();
			}
			else if (m_specification.m_existing_images.contains(attachment_index))
			{
				arc<image_2d> existing_image = m_specification.m_existing_images.at(attachment_index);
				KB_CORE_ASSERT(backend::util::IsDepthFormat(existing_image->get_specification().format), "Trying to attach non-depth image as depth attachment");
				m_depth_attachment_image = existing_image;
			}
			else
			{
				arc<vulkan_image_2d> depth_attachment_image = m_depth_attachment_image.As<vulkan_image_2d>();
				auto& spec = depth_attachment_image->get_specification();
				spec.width = m_width;
				spec.height = m_height;
				depth_attachment_image->RT_Invalidate(); // Create immediately
			}

			VkAttachmentDescription& attachment_description = attachment_descriptions.emplace_back();
			attachment_description.flags = 0;
			attachment_description.format = util::VulkanImageFormat(attachment_spec.format);
			attachment_description.samples = VK_SAMPLE_COUNT_1_BIT;
            attachment_description.loadOp = utils::get_vk_attachment_load_op(m_specification, attachment_spec);
			attachment_description.storeOp = VK_ATTACHMENT_STORE_OP_STORE; // TODO: if sampling, needs to be store (otherwise DONT_CARE is fine)
			attachment_description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			attachment_description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            attachment_description.initialLayout = attachment_description.loadOp == VK_ATTACHMENT_LOAD_OP_CLEAR ?
                VK_IMAGE_LAYOUT_UNDEFINED : VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
            // #TODO Separate layouts requires a "separate layouts" flag to be enabled
			if (attachment_spec.format == image_format_t::DEPTH24STENCIL8 || true)
			{
                // TODO: if not sampling
				attachment_description.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
                // TODO: if sampling
				attachment_description.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
				depth_attachment_reference = {
				    attachment_index,
				    VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
				};
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

			arc<vulkan_image_2d> color_attachment;
			if (m_specification.m_existing_frame_buffer)
			{
				arc<vulkan_frame_buffer> existing_frame_buffer = 
                    m_specification.m_existing_frame_buffer.As<vulkan_frame_buffer>();
				arc<image_2d> existingImage = existing_frame_buffer->get_image(attachment_index);
				color_attachment = m_attachment_images.emplace_back(existingImage).As<vulkan_image_2d>();
			}
			else if (m_specification.m_existing_images.contains(attachment_index))
			{
				arc<image_2d> existing_image = m_specification.m_existing_images[attachment_index];
				KB_CORE_ASSERT(!backend::util::IsDepthFormat(existing_image->get_specification().format), "Trying to attach depth image as color attachment");
				color_attachment = existing_image.As<vulkan_image_2d>();
				m_attachment_images[attachment_index] = existing_image;
			}
			else
			{
				if (create_images)
				{
					image_specification_t spec;
					spec.format = attachment_spec.format;
					spec.usage = image_usage_t::Attachment;
					spec.width = m_width;
					spec.height = m_height;
                    spec.m_transfer = m_specification.m_transfer;
					color_attachment = m_attachment_images.emplace_back(image_2d::create(spec)).As<vulkan_image_2d>();
                    KB_CORE_ASSERT(false, "Framebuffer attachment image should already be created!");
				}
				else
				{
					arc<image_2d> image = m_attachment_images[attachment_index];
					image_specification_t& spec = image->get_specification();
					spec.width = m_width;
					spec.height = m_height;
					color_attachment = image.As<vulkan_image_2d>();
                    if (!color_attachment->get_specification().deinterleaved)
                        color_attachment->RT_Invalidate(); // Create immediately
                    else if (color_attachment->get_specification().layers == 1)
                        color_attachment->RT_Invalidate();
					else if (attachment_index == 0 && m_specification.m_existing_image_layers[0] == 0)// Only invalidate the first layer from only the first framebuffer
					{
						color_attachment->RT_Invalidate(); // Create immediately
						color_attachment->RT_CreatePerSpecificLayerImageViews(m_specification.m_existing_image_layers);
					}
					else if (attachment_index == 0)
					{
						color_attachment->RT_CreatePerSpecificLayerImageViews(m_specification.m_existing_image_layers);
					}

#if 0
					if (image.As<VulkanImage2D>()->get_vk_image_info_descriptor().imageLayout == VK_IMAGE_LAYOUT_UNDEFINED)
						color_attachment->RT_Invalidate();
#endif
				}
			}

			VkAttachmentDescription& attachment_description = attachment_descriptions.emplace_back();
			attachment_description.flags = 0;
			attachment_description.format = util::VulkanImageFormat(attachment_spec.format);
			attachment_description.samples = VK_SAMPLE_COUNT_1_BIT;
            attachment_description.loadOp = utils::get_vk_attachment_load_op(m_specification, attachment_spec);
			attachment_description.storeOp = VK_ATTACHMENT_STORE_OP_STORE; // TODO: if sampling, needs to be store (otherwise DONT_CARE is fine)
			attachment_description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			attachment_description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            attachment_description.initialLayout = attachment_description.loadOp == VK_ATTACHMENT_LOAD_OP_CLEAR ?
                VK_IMAGE_LAYOUT_UNDEFINED : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			attachment_description.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

			const auto& clear_color = m_specification.m_clear_color;
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

	if (!m_attachment_images.empty())
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

	// Create the actual render pass
	VkRenderPassCreateInfo render_pass_info = {};
	render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	render_pass_info.attachmentCount = static_cast<uint32_t>(attachment_descriptions.size());
	render_pass_info.pAttachments = attachment_descriptions.data();
	render_pass_info.subpassCount = 1;
	render_pass_info.pSubpasses = &subpass_description;
	render_pass_info.dependencyCount = static_cast<uint32_t>(dependencies.size());
	render_pass_info.pDependencies = dependencies.data();

	if (vkCreateRenderPass(device, &render_pass_info, nullptr, &m_vk_render_pass) != VK_SUCCESS)
		KB_CORE_ASSERT(false, "Vulkan failed to create render pass");

	std::vector<VkImageView> attachments(m_attachment_images.size());
	for (uint32_t i = 0; i < m_attachment_images.size(); i++)
	{
		arc<vulkan_image_2d> image = m_attachment_images[i].As<vulkan_image_2d>();
		if (image->get_specification().deinterleaved)
		{
			attachments[i] = image->GetLayerImageView(m_specification.m_existing_image_layers[i]);
			KB_CORE_ASSERT(attachments[i], "Attachment invalid!");
		}
		else
		{
			attachments[i] = image->get_vk_image_info().image_view;
			KB_CORE_ASSERT(attachments[i], "Attachment invalid!");
		}
	}

	if (m_depth_attachment_image)
	{
		arc<vulkan_image_2d> image = m_depth_attachment_image.As<vulkan_image_2d>();
		if (m_specification.m_existing_image)
		{
			KB_CORE_ASSERT(m_specification.m_existing_image_layers.size() == 1, "Depth attachments do not support deinterleaving");
			attachments.emplace_back(image->GetLayerImageView(m_specification.m_existing_image_layers[0]));
			KB_CORE_ASSERT(attachments.back(), "error");
		}
		else
		{
			attachments.emplace_back(image->get_vk_image_info().image_view);
			KB_CORE_ASSERT(attachments.back(), "error");
		}
	}

	VkFramebufferCreateInfo frame_buffer_create_info = {};
	frame_buffer_create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	frame_buffer_create_info.renderPass = m_vk_render_pass;
	frame_buffer_create_info.attachmentCount = static_cast<uint32_t>(attachments.size());
	frame_buffer_create_info.pAttachments = attachments.data();
	frame_buffer_create_info.width = m_width;
	frame_buffer_create_info.height = m_height;
	frame_buffer_create_info.layers = 1;

	if (vkCreateFramebuffer(device, &frame_buffer_create_info, nullptr, &m_framebuffer) != VK_SUCCESS)
		KB_CORE_ASSERT(false, "Vulkan failed to create frame buffer!");
}

} // end namespace kb::render::backend::vk
