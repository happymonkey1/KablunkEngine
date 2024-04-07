#include "kablunkpch.h"
#include "Platform/Vulkan/vulkan_descriptor_set_manager.h"

#include "Kablunk/Core/Application.h"
#include "Kablunk/Renderer/Renderer.h"
#include "Kablunk/Renderer/RenderCommand.h"
#include "Kablunk/Renderer/RenderCommand2D.h"
#include "Platform/Vulkan/VulkanContext.h"
#include "Platform/Vulkan/VulkanStorageBuffer.h"
#include "Platform/Vulkan/VulkanStorageBufferSet.h"
#include "Platform/Vulkan/VulkanTexture.h"
#include "Platform/Vulkan/VulkanUniformBuffer.h"
#include "Platform/Vulkan/VulkanUniformBufferSet.h"
#include "Platform/Vulkan/vulkan_api.h"
#include "Platform/Vulkan/vulkan_core.h"

namespace kb::render
{ // start namespace kb::render

namespace util
{ // start namespace ::util

inline auto get_default_resource_type(VkDescriptorType p_type) noexcept -> render_pass_resource_type_t
{
    switch (p_type)
    {
    case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
    case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
        return render_pass_resource_type_t::texture_2d;
    case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
        return render_pass_resource_type_t::image_2d;
    case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
        return render_pass_resource_type_t::uniform_buffer;
    case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
        return render_pass_resource_type_t::storage_buffer;
    default:
        KB_CORE_ASSERT(
            false,
            "[vulkan_descriptor_set_manager]: Unhandled VkDescriptorType in get_default_resource_type!"
        );
    }

    return render_pass_resource_type_t::none;
}

} // end namespace ::util

vulkan_descriptor_set_manager::vulkan_descriptor_set_manager(const vulkan_descriptor_set_manager& p_other) noexcept
    : m_specification{ p_other.m_specification }
{
    init();
    m_input_resources = p_other.m_input_resources;
    bake();
}

vulkan_descriptor_set_manager::vulkan_descriptor_set_manager(vulkan_descriptor_set_manager&& p_other) noexcept
{
    std::swap(m_specification, p_other.m_specification);
    std::swap(m_input_declarations, p_other.m_input_declarations);
    std::swap(m_invalidated_input_resources, p_other.m_invalidated_input_resources);
    std::swap(m_input_resources, p_other.m_input_resources);
    std::swap(m_descriptor_sets, p_other.m_descriptor_sets);
    std::swap(m_write_descriptor_map, p_other.m_write_descriptor_map);
    std::swap(m_descriptor_pool, p_other.m_descriptor_pool);
}

vulkan_descriptor_set_manager::vulkan_descriptor_set_manager(descriptor_set_manager_specification p_spec) noexcept
    : m_specification{ std::move(p_spec) }
{
    init();
}

#if 0
auto vulkan_descriptor_set_manager::set_input(
    std::string_view p_name,
    const ref<UniformBuffer>& p_uniform_buffer
) noexcept -> vulkan_descriptor_set_manager&
{
    if (const auto* decl = get_input_declaration(p_name))
        m_input_resources.at(decl->m_set).at(decl->m_binding).set(p_uniform_buffer);
    else
    {
        log::core::warn(
            log::logger_tag_t::renderer,
            "[Render Pass {}]: Input {} not found!",
            m_specification.m_debug_name,
            p_name
        );
    }

    return *this;
}

auto vulkan_descriptor_set_manager::set_input(
    std::string_view p_name,
    const ref<UniformBufferSet>& p_uniform_buffer_set
) noexcept -> vulkan_descriptor_set_manager&
{
    if (const auto* decl = get_input_declaration(p_name))
        m_input_resources.at(decl->m_set).at(decl->m_binding).set(p_uniform_buffer_set);
    else
    {
        log::core::warn(
            log::logger_tag_t::renderer,
            "[Render Pass {}]: Input {} not found!",
            m_specification.m_debug_name,
            p_name
        );
    }

    return *this;
}

auto vulkan_descriptor_set_manager::set_input(
    std::string_view p_name,
    const ref<StorageBuffer>& p_storage_buffer
) noexcept -> vulkan_descriptor_set_manager&
{
    if (const auto* decl = get_input_declaration(p_name))
        m_input_resources.at(decl->m_set).at(decl->m_binding).set(p_storage_buffer);
    else
    {
        log::core::warn(
            log::logger_tag_t::renderer,
            "[Render Pass {}]: Input {} not found!",
            m_specification.m_debug_name,
            p_name
        );
    }

    return *this;
}

auto vulkan_descriptor_set_manager::set_input(
    std::string_view p_name,
    const ref<StorageBufferSet>& p_storage_buffer_set
) noexcept -> vulkan_descriptor_set_manager&
{
    if (const auto* decl = get_input_declaration(p_name))
        m_input_resources.at(decl->m_set).at(decl->m_binding).set(p_storage_buffer_set);
    else
    {
        log::core::warn(
            log::logger_tag_t::renderer,
            "[Render Pass {}]: Input {} not found!",
            m_specification.m_debug_name,
            p_name
        );
    }

    return *this;
}

auto vulkan_descriptor_set_manager::set_input(
    std::string_view p_name,
    const ref<Texture2D>& p_texture_2d,
    u32 p_index /*= 0*/
) noexcept -> vulkan_descriptor_set_manager&
{
    if (const auto* decl = get_input_declaration(p_name))
        m_input_resources.at(decl->m_set).at(decl->m_binding).set(p_texture_2d, p_index);
    else
    {
        log::core::warn(
            log::logger_tag_t::renderer,
            "[Render Pass {}]: Input {} not found!",
            m_specification.m_debug_name,
            p_name
        );
    }

    return *this;
}

auto vulkan_descriptor_set_manager::set_input(
    std::string_view p_name,
    const ref<Image2D>& p_image_2d
) noexcept -> vulkan_descriptor_set_manager&
{
    if (const auto* decl = get_input_declaration(p_name))
        m_input_resources.at(decl->m_set).at(decl->m_binding).set(p_image_2d);
    else
    {
        log::core::warn(
            log::logger_tag_t::renderer,
            "[Render Pass {}]: Input {} not found!",
            m_specification.m_debug_name,
            p_name
        );
    }

    return *this;
}
#endif

auto vulkan_descriptor_set_manager::is_invalidated(u32 p_set, u32 p_binding) const noexcept -> bool
{
    if (m_invalidated_input_resources.contains(p_set))
    {
        const auto& resources = m_invalidated_input_resources.at(p_set);
        return resources.contains(p_binding);
    }

    return false;
}

auto vulkan_descriptor_set_manager::validate() noexcept -> bool
{
    const auto& shader_descriptor_sets = m_specification.m_shader->GetShaderDescriptorSets();

    for (u32 set = m_specification.m_start_set; set <= m_specification.m_end_set; ++set)
    {
        if (set >= shader_descriptor_sets.size())
            break;

        // no descriptors in this set
        if (!shader_descriptor_sets[set])
            continue;

        if (!m_input_resources.contains(set))
        {
            log::core::error(
                log::logger_tag_t::renderer,
                "[Render Pass {}] No input resources for set {}?",
                m_specification.m_debug_name,
                set
            );
            return false;
        }

        const auto& set_input_resources = m_input_resources.at(set);
        const auto& shader_descriptor = shader_descriptor_sets[set];
        for (auto&& [name, write_descriptor] : shader_descriptor.write_descriptor_sets)
        {
            const u32 binding = write_descriptor.dstBinding;
            if (!set_input_resources.contains(binding))
            {
                log::core::error(
                    log::logger_tag_t::renderer,
                    "[Render Pass {}] No input resources for set {}.{}?",
                    m_specification.m_debug_name,
                    set,
                    binding
                );
                log::core::error(
                    log::logger_tag_t::renderer,
                    "  Required resource is {} ({})",
                    std::string_view{ name },
                    static_cast<i32>(write_descriptor.descriptorType)
                );
                return false;
            }

            const auto& resource = set_input_resources.at(binding);
            if (!is_input_compatible(resource.m_type, write_descriptor.descriptorType))
            {
                log::core::error(
                    log::logger_tag_t::renderer,
                    "[Render Pass {}] Wrong type for resource. Expected {}, found {}!",
                    m_specification.m_debug_name,
                    static_cast<std::underlying_type_t<render_pass_resource_type_t>>(resource.m_type),
                    static_cast<i32>(write_descriptor.descriptorType)
                );
                return false;
            }

            if (resource.m_type != render_pass_resource_type_t::image_2d && resource.m_input[0] == ref<RefCounted>{})
            {
                log::core::error(
                    log::logger_tag_t::renderer,
                    "[Render Pass {}] Resource is null! {} ({}.{})",
                    m_specification.m_debug_name,
                    name,
                    set,
                    binding
                );
                return false;
            }
        }
    }

    return true;
}

auto vulkan_descriptor_set_manager::bake() noexcept -> void
{
    if (!validate())
    {
        log::core::error(
            log::logger_tag_t::renderer,
            "[Render Pass {}] Validation failed!",
            m_specification.m_debug_name
        );
        return;
    }

    VkDescriptorPoolSize pool_sizes[] = {
        { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
        { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
        { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
    };

    constexpr u32 pool_size_count = sizeof(pool_sizes) / sizeof(VkDescriptorPoolSize);

    const VkDescriptorPoolCreateInfo pool_create_info{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
        .maxSets = pool_size_count * render::get_frames_in_flights(),
        .poolSizeCount = pool_size_count,
        .pPoolSizes = pool_sizes
    };

    const VkDevice device = VulkanContext::Get()->GetDevice()->GetVkDevice();
    KB_VK_CHECK_RESULT(vkCreateDescriptorPool(device, &pool_create_info, nullptr, &m_descriptor_pool));

    const auto buffer_sets = has_buffer_sets();
    const u32 descriptor_set_count = render::get_frames_in_flights();

    if (m_descriptor_sets.empty())
    {
        for (u32 i = 0; i < descriptor_set_count; ++i)
        {
            m_descriptor_sets.emplace_back();
        }
    }

    for (auto& descriptor_set : m_descriptor_sets)
        descriptor_set.clear();

    for (const auto& [set, data_for_set] : m_input_resources)
    {
        const u32 descriptor_count_in_set = buffer_sets.contains(set) ? descriptor_set_count : 1;
        for (u32 frame_index = 0; frame_index < descriptor_set_count; ++frame_index)
        {
            const auto vk_descriptor_set_layout = m_specification.m_shader->GetDescriptorSetLayout(set);
            const auto descriptor_set_allocate_info = kb::vk::get_descriptor_set_alloc_info(
                &vk_descriptor_set_layout,
                descriptor_count_in_set,
                m_descriptor_pool
            );

            VkDescriptorSet vk_descriptor_set;
            KB_VK_CHECK_RESULT(vkAllocateDescriptorSets(device, &descriptor_set_allocate_info, &vk_descriptor_set));
            m_descriptor_sets[frame_index].emplace_back(vk_descriptor_set);

            auto& write_descriptor_map = m_write_descriptor_map.at(frame_index).at(set);
            std::vector<std::vector<VkDescriptorImageInfo>> image_info_storage{};
            // #TODO reserve?
            u32 image_info_storage_index = 0;

            for (const auto& [binding, input] : data_for_set)
            {
                auto& stored_write_descriptor = write_descriptor_map.at(binding);

                auto& vk_write_descriptor = stored_write_descriptor.m_write_descriptor_set;
                vk_write_descriptor.dstSet = vk_descriptor_set;

                switch (input.m_type)
                {
                case render_pass_resource_type_t::uniform_buffer:
                {
                    auto buffer = input.m_input[0].As<VulkanUniformBuffer>();
                    vk_write_descriptor.pBufferInfo = &buffer->GetDescriptorBufferInfo();
                    stored_write_descriptor.m_resource_handles[0] = vk_write_descriptor.pBufferInfo->buffer;

                    // defer if resource does not exist yet
                    if (vk_write_descriptor.pBufferInfo->buffer == nullptr)
                        m_invalidated_input_resources[set][binding] = input;

                    break;
                }
                case render_pass_resource_type_t::uniform_buffer_set:
                {
                    auto buffer = input.m_input[0].As<VulkanUniformBufferSet>();
                    // #TODO validate if this is correct
                    vk_write_descriptor.pBufferInfo = &buffer->Get(0, 0, frame_index).As<VulkanUniformBuffer>()->GetDescriptorBufferInfo();
                    stored_write_descriptor.m_resource_handles[0] = vk_write_descriptor.pBufferInfo->buffer;

                    // defer if resource does not exist yet
                    if (vk_write_descriptor.pBufferInfo->buffer == nullptr)
                        m_invalidated_input_resources[set][binding] = input;

                    break;
                }
                case render_pass_resource_type_t::storage_buffer:
                {
                    auto buffer = input.m_input[0].As<VulkanStorageBuffer>();
                    vk_write_descriptor.pBufferInfo = &buffer->GetVkDescriptorInfo();
                    stored_write_descriptor.m_resource_handles[0] = vk_write_descriptor.pBufferInfo->buffer;

                    // defer if resource does not exist yet
                    if (vk_write_descriptor.pBufferInfo->buffer == nullptr)
                        m_invalidated_input_resources[set][binding] = input;

                    break;
                }
                case render_pass_resource_type_t::storage_buffer_set:
                {
                    auto buffer = input.m_input[0].As<VulkanStorageBufferSet>();
                    // #TODO validate if this is correct
                    vk_write_descriptor.pBufferInfo = &buffer->Get(0, 0, frame_index).As<VulkanStorageBuffer>()->GetVkDescriptorInfo();
                    stored_write_descriptor.m_resource_handles[0] = vk_write_descriptor.pBufferInfo->buffer;

                    // defer if resource does not exist yet
                    if (vk_write_descriptor.pBufferInfo->buffer == nullptr)
                        m_invalidated_input_resources[set][binding] = input;

                    break;
                }
                case render_pass_resource_type_t::texture_2d:
                {
                    if (input.m_input.size() > 1)
                    {
                        image_info_storage.emplace_back(input.m_input.size());
                        for (size_t i = 0; i < input.m_input.size(); ++i)
                        {
                            auto texture = input.m_input[i].As<VulkanTexture2D>();
                            image_info_storage[image_info_storage_index][i] = texture->GetVulkanDescriptorInfo();
                        }

                        vk_write_descriptor.pImageInfo = image_info_storage[image_info_storage_index].data();
                        image_info_storage_index++;
                    }
                    else
                    {
                        auto texture = input.m_input[0].As<VulkanTexture2D>();
                        vk_write_descriptor.pImageInfo = image_info_storage[image_info_storage_index].data();
                    }
                    stored_write_descriptor.m_resource_handles[0] = vk_write_descriptor.pImageInfo->imageView;

                    // defer if resource does not exist yet
                    if (vk_write_descriptor.pImageInfo->imageView == nullptr)
                        m_invalidated_input_resources[set][binding] = input;

                    break;
                }
                case render_pass_resource_type_t::texture_3d:
                {
                    KB_CORE_ASSERT(false, "[vulkan_write_descriptor_set]: Not implemented!");
                    break;
                }
                case render_pass_resource_type_t::image_2d:
                {
                    auto image = input.m_input[0].As<VulkanImage2D>();

                    // defer if resource does not exist yet
                    if (image == ref<VulkanImage2D>{})
                    {
                        m_invalidated_input_resources[set][binding] = input;
                        break;
                    }

                    vk_write_descriptor.pImageInfo = &image->GetDescriptor();
                    stored_write_descriptor.m_resource_handles[0] = vk_write_descriptor.pImageInfo->imageView;

                    // defer if resource does not exist yet
                    if (vk_write_descriptor.pImageInfo->imageView == nullptr)
                        m_invalidated_input_resources[set][binding] = input;

                    break;
                }
                case render_pass_resource_type_t::none: [[fallthrough]];
                default:
                {
                    KB_CORE_ASSERT(
                        false,
                        "[vulkan_write_descriptor_set]: Invalid or unhandled render_pass_resource_type_t {}!",
                        static_cast<std::underlying_type_t<render_pass_resource_type_t>>(input.m_type)
                    );
                    return;
                }
                }
            }
        }
    }
}

auto vulkan_descriptor_set_manager::has_buffer_sets() noexcept -> std::set<u32>
{
    std::set<u32> sets{};

    for (const auto& [set, resources] : m_input_resources)
    {
        for (const auto& input : resources | std::views::values)
        {
            if (input.m_type == render_pass_resource_type_t::uniform_buffer_set ||
                input.m_type == render_pass_resource_type_t::storage_buffer_set)
            {
                sets.insert(set);
                break;
            }
        }
    }

    return sets;
}

auto vulkan_descriptor_set_manager::rt_invalidate_and_update() noexcept -> void
{
    KB_PROFILE_SCOPE_NAMED("vulkan_descriptor_set_manager::rt_invalidate_and_update");

    log::core::info(
        log::logger_tag_t::renderer,
        "[Render pass {}] invalidating and updating resources",
        m_specification.m_debug_name
    );

    const auto frame_index = render::rt_get_current_frame_index();

    // map invalid resources to update
    for (const auto& [set, inputs] : m_input_resources)
    {
        for (const auto& [binding, input] : inputs)
        {
            switch (input.m_type)
            {
            case render_pass_resource_type_t::uniform_buffer:
            {
                const VkDescriptorBufferInfo& buffer_info =
                    input.m_input[0].As<VulkanUniformBuffer>()->GetDescriptorBufferInfo();

                if (buffer_info.buffer !=
                    m_write_descriptor_map[frame_index].at(set).at(binding).m_resource_handles[0])
                {
                    m_invalidated_input_resources[set][binding] = input;
                }

                break;
            }
            case render_pass_resource_type_t::uniform_buffer_set:
            {
                const VkDescriptorBufferInfo& buffer_info =
                    input.m_input[0].As<VulkanUniformBufferSet>()->Get(0, 0, frame_index)
                        .As<VulkanUniformBuffer>()->GetDescriptorBufferInfo();

                if (buffer_info.buffer !=
                    m_write_descriptor_map[frame_index].at(set).at(binding).m_resource_handles[0])
                {
                    m_invalidated_input_resources[set][binding] = input;
                }

                break;
            }
            case render_pass_resource_type_t::storage_buffer:
            {
                const VkDescriptorBufferInfo& buffer_info =
                    input.m_input[0].As<VulkanStorageBuffer>()->GetVkDescriptorInfo();

                if (buffer_info.buffer !=
                    m_write_descriptor_map[frame_index].at(set).at(binding).m_resource_handles[0])
                {
                    m_invalidated_input_resources[set][binding] = input;
                }

                break;
            }
            case render_pass_resource_type_t::storage_buffer_set:
            {
                const VkDescriptorBufferInfo& buffer_info =
                    input.m_input[0].As<VulkanStorageBufferSet>()->Get(0, 0, frame_index)
                    .As<VulkanStorageBuffer>()->GetVkDescriptorInfo();

                if (buffer_info.buffer !=
                    m_write_descriptor_map[frame_index].at(set).at(binding).m_resource_handles[0])
                {
                    m_invalidated_input_resources[set][binding] = input;
                }

                break;
            }
            case render_pass_resource_type_t::texture_2d:
            {
                for (size_t i = 0; i < input.m_input.size(); ++i)
                {
                    const auto& image_info = input.m_input[i].As<VulkanTexture2D>()->GetVulkanDescriptorInfo();
                    if (image_info.imageView != m_write_descriptor_map[frame_index][set][binding].m_resource_handles[i])
                    {
                        m_invalidated_input_resources[set][binding] = input;
                        break;
                    }
                }

                break;
            }
            case render_pass_resource_type_t::texture_3d:
            {
                KB_CORE_ASSERT(false, "Not implemented!");
                break;
            }
            case render_pass_resource_type_t::image_2d:
            {
                const auto& image_info = input.m_input[0].As<VulkanImage2D>()->GetDescriptor();
                if (image_info.imageView != m_write_descriptor_map[frame_index][set][binding].m_resource_handles[0])
                {
                    m_invalidated_input_resources[set][binding] = input;
                }

                break;
            }
            case render_pass_resource_type_t::none: [[fallthrough]];
            default:
                KB_CORE_ASSERT(
                    false,
                    "Unhandled render_pass_resource_type {}!",
                    static_cast<std::underlying_type_t<render_pass_resource_type_t>>(input.m_type)
                );
                break;
            }
        }
    }

    // exit early if there are no invalidated resources
    if (m_invalidated_input_resources.empty())
        return;

    auto buffer_sets = has_buffer_sets();
    const auto descriptor_set_count = render::get_frames_in_flights();

    // #TODO handle invalid buffers/images
    // iterate invalid inputs and call VkUpdateDescriptorSets with write descriptors
    for (const auto& [set, data_for_set] : m_invalidated_input_resources)
    {
        const auto descriptor_count_in_set = buffer_sets.contains(set) ? descriptor_set_count : 1ul;

        std::vector<VkWriteDescriptorSet> write_descriptors_to_update{};
        write_descriptors_to_update.reserve(data_for_set.size());
        std::vector<std::vector<VkDescriptorImageInfo>> image_info_storage{};
        u32 image_info_storage_index = 0;
        for (const auto& [binding, input] : data_for_set)
        {
            auto& write_descriptor = m_write_descriptor_map[frame_index][set][binding];
            auto& vk_write_descriptor_set = write_descriptor.m_write_descriptor_set;

            switch (input.m_type)
            {
            case render_pass_resource_type_t::uniform_buffer:
            {
                auto buffer = input.m_input[0].As<VulkanUniformBuffer>();
                vk_write_descriptor_set.pBufferInfo = &buffer->GetDescriptorBufferInfo();
                write_descriptor.m_resource_handles[0] = vk_write_descriptor_set.pBufferInfo->buffer;
                break;
            }
            case render_pass_resource_type_t::uniform_buffer_set:
            {
                auto buffer = input.m_input[0].As<VulkanUniformBufferSet>();
                vk_write_descriptor_set.pBufferInfo = &buffer->Get(0, 0, frame_index)
                    .As<VulkanUniformBuffer>()->GetDescriptorBufferInfo();
                write_descriptor.m_resource_handles[0] = vk_write_descriptor_set.pBufferInfo->buffer;
                break;
            }
            case render_pass_resource_type_t::storage_buffer:
            {
                auto buffer = input.m_input[0].As<VulkanStorageBuffer>();
                vk_write_descriptor_set.pBufferInfo = &buffer->GetVkDescriptorInfo();
                write_descriptor.m_resource_handles[0] = vk_write_descriptor_set.pBufferInfo->buffer;
                break;
            }
            case render_pass_resource_type_t::storage_buffer_set:
            {
                auto buffer = input.m_input[0].As<VulkanStorageBufferSet>();
                vk_write_descriptor_set.pBufferInfo = &buffer->Get(0, 0, frame_index)
                    .As<VulkanStorageBuffer>()->GetVkDescriptorInfo();
                write_descriptor.m_resource_handles[0] = vk_write_descriptor_set.pBufferInfo->buffer;
                break;
            }
            case render_pass_resource_type_t::texture_2d:
            {
                if (input.m_input.size() > 1)
                {
                    image_info_storage.emplace_back(input.m_input.size());
                    for (size_t i = 0; i < input.m_input.size(); ++i)
                    {
                        auto texture = input.m_input[i].As<VulkanTexture2D>();
                        image_info_storage[image_info_storage_index][i] = texture->GetVulkanDescriptorInfo();
                        write_descriptor.m_resource_handles[i] =
                            image_info_storage[image_info_storage_index][i].imageView;
                    }

                    vk_write_descriptor_set.pImageInfo = image_info_storage[image_info_storage_index++].data();
                }
                else
                {
                    auto texture = input.m_input[0].As<VulkanTexture2D>();
                    vk_write_descriptor_set.pImageInfo = &texture->GetVulkanDescriptorInfo();
                    write_descriptor.m_resource_handles[0] = vk_write_descriptor_set.pImageInfo->imageView;
                }

                break;
            }
            case render_pass_resource_type_t::texture_3d:
            {
                KB_CORE_ASSERT(false, "not implemented!");
                break;
            }
            case render_pass_resource_type_t::image_2d:
            {
                auto image = input.m_input[0].As<VulkanImage2D>();
                vk_write_descriptor_set.pImageInfo = &image->GetDescriptor();
                KB_CORE_ASSERT(
                    vk_write_descriptor_set.pImageInfo->imageView,
                    "[vulkan_descriptor_set_manager]: VkDescriptorImageInfo {} is null?",
                    static_cast<const void*>(vk_write_descriptor_set.pImageInfo)
                );
                write_descriptor.m_resource_handles[0] = vk_write_descriptor_set.pImageInfo->imageView;
                break;
            }
            case render_pass_resource_type_t::none: [[fallthrough]];
            default:
                KB_CORE_ASSERT(
                    false,
                    "Unhandled render_pass_resource_type_t {}!",
                    static_cast<std::underlying_type_t<render_pass_resource_type_t>>(input.m_type)
                );
                break;
            }

            write_descriptors_to_update.emplace_back(vk_write_descriptor_set);
        }

        log::core::info(
            log::logger_tag_t::renderer,
            "vulkan_descriptor_set_manager::invalidate_and_update ({}): updating {} descriptors in set {} (frame_index={})",
            m_specification.m_debug_name,
            write_descriptors_to_update.size(),
            set,
            frame_index
        );
        const auto device = VulkanContext::Get()->GetDevice()->GetVkDevice();
        vkUpdateDescriptorSets(
            device,
            static_cast<u32>(write_descriptors_to_update.size()),
            write_descriptors_to_update.data(),
            0,
            nullptr
        );
    }

    m_invalidated_input_resources.clear();
}

auto vulkan_descriptor_set_manager::has_descriptor_sets() const noexcept -> bool
{
    return !m_descriptor_sets.empty() && !m_descriptor_sets[0].empty();
}

auto vulkan_descriptor_set_manager::get_first_set_index() const noexcept -> std::optional<u32>
{
    if (m_invalidated_input_resources.empty())
        return std::nullopt;

    return m_input_resources.begin()->first;
}

auto vulkan_descriptor_set_manager::get_descriptor_sets(
    u32 frame_index
) const noexcept -> const std::vector<VkDescriptorSet>&
{
    KB_CORE_ASSERT(
        !m_descriptor_sets.empty(),
        "[vulkan_descriptor_set_manager]: Trying to get descriptor set but set at frame_index={} is empty!",
        frame_index
    );

    if (frame_index > 0 && m_descriptor_sets.size() == 1)
    {
        return m_descriptor_sets[0];
    }

    KB_CORE_ASSERT(
        frame_index < m_descriptor_sets.size(),
        "[vulkan_descriptor_set_manager]: Trying to access descriptor set {}, out of set size {}!",
        frame_index,
        m_descriptor_sets.size()
    );
    return m_descriptor_sets[frame_index];
}

auto vulkan_descriptor_set_manager::is_input_valid(std::string_view p_name) const noexcept -> bool
{
    return m_input_declarations.contains(std::string{ p_name });
}

auto vulkan_descriptor_set_manager::get_input_declaration(
    std::string_view p_name
) const noexcept -> const render_pass_input_declaration*
{
    const std::string name_str{ p_name };
    if (!m_input_declarations.contains(name_str))
        return nullptr;

    return &m_input_declarations.at(name_str);
}

auto vulkan_descriptor_set_manager::operator=(const vulkan_descriptor_set_manager& p_other) noexcept -> vulkan_descriptor_set_manager&
{
    m_specification = p_other.m_specification;

    init();
    m_input_resources = p_other.m_input_resources;
    bake();

    return *this;
}

auto vulkan_descriptor_set_manager::operator=(vulkan_descriptor_set_manager&& p_other) noexcept -> vulkan_descriptor_set_manager&
{
    std::swap(m_specification, p_other.m_specification);
    std::swap(m_input_declarations, p_other.m_input_declarations);
    std::swap(m_invalidated_input_resources, p_other.m_invalidated_input_resources);
    std::swap(m_input_resources, p_other.m_input_resources);
    std::swap(m_descriptor_sets, p_other.m_descriptor_sets);
    std::swap(m_write_descriptor_map, p_other.m_write_descriptor_map);
    std::swap(m_descriptor_pool, p_other.m_descriptor_pool);

    return *this;
}

auto vulkan_descriptor_set_manager::init() noexcept -> void
{
    log::core::info(
        log::logger_tag_t::renderer,
        "[vulkan_descriptor_set_manager]: Initializing."
    );

    const auto& shader_descriptor_sets = m_specification.m_shader->GetShaderDescriptorSets();
    const auto frames_in_flight = render::get_frames_in_flights();
    m_write_descriptor_map.resize(frames_in_flight);

    for (u32 set = m_specification.m_start_set; set <= m_specification.m_end_set; ++set)
    {
        if (set >= shader_descriptor_sets.size())
            break;

        const auto& shader_descriptor = shader_descriptor_sets[set];
        for (auto&& [name, write_descriptor] : shader_descriptor.write_descriptor_sets)
        {
            const auto binding = write_descriptor.dstBinding;
            auto& input_decl = m_input_declarations[name];
            input_decl.m_type = vk_descriptor_type_to_render_pass_input_type(write_descriptor.descriptorType);
            input_decl.m_set = set;
            input_decl.m_binding = binding;
            input_decl.m_name = name;
            input_decl.m_count = write_descriptor.descriptorCount;

            if (m_specification.m_default_resources || true)
            {
                auto& input = m_input_resources[set][binding];
                input.m_input.resize(write_descriptor.descriptorCount);
                input.m_type = util::get_default_resource_type(write_descriptor.descriptorType);

                // set default textures
                if (input_decl.m_type == render_pass_input_type_t::image_sampler_2d)
                {
                    for (auto& texture : input.m_input)
                        texture = Application::Get().get_renderer_2d()->get_white_texture();
                }
                // #TODO default texture_3d
            }

            for (u32 frame_index = 0; frame_index < frames_in_flight; ++frame_index)
            {
                m_write_descriptor_map[frame_index][set][binding] = {
                    .m_write_descriptor_set = write_descriptor,
                    .m_resource_handles = std::vector<void*>(write_descriptor.descriptorCount),
                };
            }

            if (shader_descriptor.image_samplers.contains(binding))
            {
                auto& image_sampler = shader_descriptor.image_samplers.at(binding);
                const auto dimension = image_sampler.m_dimension;
                KB_CORE_ASSERT(
                    dimension != 0,
                    "[vulkan_write_descriptor_manager]: image sampler for '{}' dimension not set?",
                    image_sampler.name
                );

                if (write_descriptor.descriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER ||
                    write_descriptor.descriptorType == VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE)
                {
                    switch (dimension)
                    {
                    case 1:
                        input_decl.m_type = render_pass_input_type_t::image_sampler_1d;
                        break;
                    case 2:
                        input_decl.m_type = render_pass_input_type_t::image_sampler_2d;
                        break;
                    case 3:
                        input_decl.m_type = render_pass_input_type_t::image_sample_3d;
                        break;
                    default:
                        KB_CORE_ASSERT(
                            false,
                            "[vulkan_descriptor_set_manager]: Unhandled image sampler dimension {}",
                            dimension
                        );
                    }
                }
                else if (write_descriptor.descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_IMAGE)
                {
                    switch (dimension)
                    {
                    case 1:
                        input_decl.m_type = render_pass_input_type_t::storage_image_1d;
                        break;
                    case 2:
                        input_decl.m_type = render_pass_input_type_t::storage_image_2d;
                        break;
                    case 3:
                        input_decl.m_type = render_pass_input_type_t::storage_image_3d;
                        break;
                    default:
                        KB_CORE_ASSERT(
                            false,
                            "[vulkan_descriptor_set_manager]: Unhandled storage image dimension {}",
                            dimension
                        );
                    }
                }
            }
        }
    }
}

} // end namespace kb::render
