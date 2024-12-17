#include "kablunkpch.h"

#include "Kablunk/renderer/backend/vulkan/vulkan_texture_cube.h"
#include "Kablunk/renderer/render_command.h"
#include "Kablunk/renderer/backend/vulkan/vulkan_api.h"

#include <map>

namespace kb::render::backend::vk
{ // start namespace kb::render::backend::vk

static std::map<VkImage, arc<vulkan_texture_cube>> s_texture_cube_references{};

vulkan_texture_cube::vulkan_texture_cube(
    const VkDevice p_vk_device,
    const texture_specification_t& p_specification,
    const void* p_data,
    const size_t p_size
) : m_specification{ p_specification }, m_vk_device{ p_vk_device }
{

    if (p_data)
    {
        const u32 expected_size = p_specification.m_width * p_specification.m_height * 4 * 6;
        KB_CORE_ASSERT(
            p_size == expected_size,
            "[vulkan_texture_3d]: size={} does not match expected size={}",
            p_size,
            expected_size
        );
        m_local_buffer = owning_buffer{ p_data, p_size };
    }

    invalidate();
}

vulkan_texture_cube::~vulkan_texture_cube() noexcept
{
    destroy();
}

void vulkan_texture_cube::set_data(void* data, u32 size)
{
}

u32 vulkan_texture_cube::get_mip_level_count() const noexcept
{
    KB_CORE_ASSERT(false, "[vulkan_texture_cube]: Not implemented!");
    return 0;
}

std::pair<u32, u32> vulkan_texture_cube::get_mip_size(u32 p_mip) const noexcept
{
    KB_CORE_ASSERT(false, "[vulkan_texture_cube]: Not implemented!");
    return {};
}

auto vulkan_texture_cube::create_image_view_with_single_mip(u32 p_mip) noexcept -> VkImageView
{
    KB_CORE_ASSERT(false, "[vulkan_texture_cube]: Not implemented!");
    return nullptr;
}

auto vulkan_texture_cube::generate_mips(bool p_read_only) noexcept -> void
{
}

auto vulkan_texture_cube::destroy() noexcept -> void
{
    if (!m_vk_image)
        return;

    // Defer vulkan resource destruction to renderer
    render::submit_resource_free([vk_device = m_vk_device, vk_image = m_vk_image, vk_alloc = m_vk_memory_allocation, vk_descriptor_image_info = m_vk_descriptor_image_info]
        {
            KB_CORE_INFO("[renderer]: Destroying vulkan_texture_3d '{}'", static_cast<const void*>(vk_image));
            vkDestroyImageView(vk_device, vk_descriptor_image_info.imageView, nullptr);
            vk::destroy_sampler(vk_descriptor_image_info.sampler);

            vulkan_allocator allocator{ "vulkan_texture_3d" };
            allocator.destroy_image(vk_image, vk_alloc);
            s_texture_cube_references.erase(vk_image);
        });

    m_vk_image = nullptr;
    m_vk_descriptor_image_info.imageView = nullptr;
    m_vk_descriptor_image_info.sampler = nullptr;
}

auto vulkan_texture_cube::invalidate() noexcept -> void
{
    destroy();

    const auto vk_format = util::get_vk_image_format(m_specification.m_format);
    const u32 mip_count = get_mip_level_count();

    VkMemoryAllocateInfo vk_memory_allocate_info{};
    vk_memory_allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;

    vulkan_allocator allocator{ "vulkan_texture_3d" };

    const VkImageCreateInfo vk_image_create_info{
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .pNext = nullptr,
        .flags = {},
        .imageType = VK_IMAGE_TYPE_2D, // why not 3d?
        .format = vk_format,
        .extent = { m_specification.m_width, m_specification.m_height, 1 },
        .mipLevels = mip_count,
        .arrayLayers = 6,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .tiling = VK_IMAGE_TILING_OPTIMAL,
        .usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = {},
        .pQueueFamilyIndices = {},
        .initialLayout = VK_IMAGE_LAYOUT_GENERAL
    };

    m_vk_memory_allocation = allocator.allocate_image(
        vk_image_create_info,
        VMA_MEMORY_USAGE_GPU_ONLY,
        m_vk_image
    );

    s_texture_cube_references[m_vk_image] = arc{ this };

    m_vk_descriptor_image_info.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
}

} // end namespace kb::render::backend::vk
