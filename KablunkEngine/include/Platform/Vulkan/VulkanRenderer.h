#pragma once
#ifndef KABLUNK_PLATFORM_VULKAN_RENDERER_H
#define KABLUNK_PLATFORM_VULKAN_RENDERER_H

#include "Kablunk/Renderer/Renderer.h"

#include <vulkan/vulkan.h>

namespace kb
{

	namespace Utils {

		void InsertImageMemoryBarrier(
			VkCommandBuffer cmd_buffer,
			VkImage image,
			VkAccessFlags src_access_mask,
			VkAccessFlags dst_access_mask,
			VkImageLayout old_image_layout,
			VkImageLayout new_image_layout,
			VkPipelineStageFlags src_stage_mask,
			VkPipelineStageFlags dst_stage_mask,
			VkImageSubresourceRange subresource_range);

		void SetImageLayout(
			VkCommandBuffer cmd_buffer,
			VkImage image,
			VkImageLayout old_image_layout,
			VkImageLayout new_image_layout,
			VkImageSubresourceRange subresourceRange,
			VkPipelineStageFlags src_stage_mask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
			VkPipelineStageFlags dst_stage_mask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);

		void SetImageLayout(
			VkCommandBuffer cmd_buffer,
			VkImage image,
			VkImageAspectFlags aspectMask,
			VkImageLayout old_image_layout,
			VkImageLayout new_image_layout,
			VkPipelineStageFlags src_stage_mask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
			VkPipelineStageFlags dst_stage_mask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);

	}

}

#endif
