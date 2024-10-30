#pragma once

#include "Kablunk/renderer/backend/image.h"
#include "Kablunk/renderer/backend/vulkan/vulkan_context.h"

#include <vulkan/vulkan.h>

namespace kb::render::backend::vk::util
{ // start namespace kb::render::backend::vk::util

void InsertImageMemoryBarrier(
    VkCommandBuffer cmd_buffer,
    VkImage image,
    VkAccessFlags src_access_mask,
    VkAccessFlags dst_access_mask,
    VkImageLayout old_image_layout,
    VkImageLayout new_image_layout,
    VkPipelineStageFlags src_stage_mask,
    VkPipelineStageFlags dst_stage_mask,
    VkImageSubresourceRange subresource_range
);

void SetImageLayout(
    VkCommandBuffer cmd_buffer,
    VkImage image,
    VkImageLayout old_image_layout,
    VkImageLayout new_image_layout,
    VkImageSubresourceRange subresourceRange,
    VkPipelineStageFlags src_stage_mask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
    VkPipelineStageFlags dst_stage_mask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT
);

void SetImageLayout(
    VkCommandBuffer cmd_buffer,
    VkImage image,
    VkImageAspectFlags aspectMask,
    VkImageLayout old_image_layout,
    VkImageLayout new_image_layout,
    VkPipelineStageFlags src_stage_mask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
    VkPipelineStageFlags dst_stage_mask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT
);

// Convert between Kablunk image format and vulkan image format
inline VkFormat VulkanImageFormat(image_format_t format)
{
    switch (format)
    {
    case image_format_t::RED32I:		   return VK_FORMAT_R32_SINT;
    case image_format_t::RED32F:          return VK_FORMAT_R32_SFLOAT;
    case image_format_t::RG16F:		   return VK_FORMAT_R16G16_SFLOAT;
    case image_format_t::RG32F:		   return VK_FORMAT_R32G32_SFLOAT;
    case image_format_t::RGBA:            return VK_FORMAT_R8G8B8A8_UNORM;
    case image_format_t::RGBA16F:         return VK_FORMAT_R16G16B16A16_SFLOAT;
    case image_format_t::RGBA32F:         return VK_FORMAT_R32G32B32A32_SFLOAT;
    case image_format_t::DEPTH32F:        return VK_FORMAT_D32_SFLOAT;
    case image_format_t::DEPTH24STENCIL8:
    {
        const auto& graphics_context = Singleton<Renderer>::get().get_graphics_context();
        const auto vulkan_context = graphics_context.as<vk::vulkan_context>();
        return vulkan_context->get_device()->get_physical_device()->GetDepthFormat();
    }
    }
    KB_CORE_ASSERT(false, "Unknown ImageFormat!");
    return VK_FORMAT_UNDEFINED;
}

} // end namespace kb::render::backend::vk::util
