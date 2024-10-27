#pragma once

#include "kablunk/renderer/backend/vulkan/vulkan_core.h"
#include "kablunk/renderer/backend/vulkan/vulkan_utils.h"

namespace kb::render::backend::vk
{ // start namespace kb::render::backend::vk

auto get_descriptor_set_alloc_info(
    const VkDescriptorSetLayout* p_layouts,
    u32 p_count = 1,
    VkDescriptorPool p_pool = nullptr
) noexcept -> VkDescriptorSetAllocateInfo;

auto create_sampler(const VkSamplerCreateInfo& p_sampler_create_info) noexcept -> VkSampler;
auto destroy_sampler(VkSampler p_sampler) noexcept -> void;

auto get_current_vk_device() noexcept -> VkDevice;

} // end namespace kb::render::backend::vk
