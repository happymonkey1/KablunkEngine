#include "kablunkpch.h"
#include "Platform/Vulkan/vulkan_descriptor_set_manager.h"

#include "Kablunk/Core/Application.h"
#include "Kablunk/Renderer/Renderer.h"
#include "Kablunk/Renderer/RenderCommand.h"
#include "Kablunk/Renderer/RenderCommand2D.h"

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

vulkan_descriptor_set_manager::vulkan_descriptor_set_manager(const descriptor_set_manager_specification& p_spec) noexcept
    : m_specification{ p_spec }
{
    init();
}

vulkan_descriptor_set_manager::~vulkan_descriptor_set_manager() noexcept
{
}

auto vulkan_descriptor_set_manager::set_input(
    std::string_view p_name,
    const ref<UniformBuffer>& p_uniform_buffer
) noexcept -> void
{
    if (const auto* decl = get_input_declaration(p_name))
        m_input_resources.at(decl->m_set).at(decl->m_binding).set(p_uniform_buffer);
    else
    {
        KB_CORE_WARN(
            "[vulkan_descriptor_set_manager]: [RenderPass {}]: Input {} not found!",
            m_specification.m_debug_name,
            p_name
        );
    }
}

auto vulkan_descriptor_set_manager::set_input(
    std::string_view p_name,
    const ref<UniformBufferSet>& p_uniform_buffer_set
) noexcept -> void
{
    if (const auto* decl = get_input_declaration(p_name))
        m_input_resources.at(decl->m_set).at(decl->m_binding).set(p_uniform_buffer_set);
    else
    {
        KB_CORE_WARN(
            "[vulkan_descriptor_set_manager]: [RenderPass {}]: Input {} not found!",
            m_specification.m_debug_name,
            p_name
        );
    }
}

auto vulkan_descriptor_set_manager::set_input(
    std::string_view p_name,
    const ref<StorageBuffer>& p_storage_buffer
) noexcept -> void
{
    if (const auto* decl = get_input_declaration(p_name))
        m_input_resources.at(decl->m_set).at(decl->m_binding).set(p_storage_buffer);
    else
    {
        KB_CORE_WARN(
            "[vulkan_descriptor_set_manager]: [RenderPass {}]: Input {} not found!",
            m_specification.m_debug_name,
            p_name
        );
    }
}

auto vulkan_descriptor_set_manager::set_input(
    std::string_view p_name,
    const ref<StorageBufferSet>& p_storage_buffer_set
) noexcept -> void
{
    if (const auto* decl = get_input_declaration(p_name))
        m_input_resources.at(decl->m_set).at(decl->m_binding).set(p_storage_buffer_set);
    else
    {
        KB_CORE_WARN(
            "[vulkan_descriptor_set_manager]: [RenderPass {}]: Input {} not found!",
            m_specification.m_debug_name,
            p_name
        );
    }
}

auto vulkan_descriptor_set_manager::set_input(
    std::string_view p_name,
    const ref<Texture2D>& p_texture_2d
) noexcept -> void
{
    if (const auto* decl = get_input_declaration(p_name))
        m_input_resources.at(decl->m_set).at(decl->m_binding).set(p_texture_2d);
    else
    {
        KB_CORE_WARN(
            "[vulkan_descriptor_set_manager]: [RenderPass {}]: Input {} not found!",
            m_specification.m_debug_name,
            p_name
        );
    }
}

auto vulkan_descriptor_set_manager::set_input(std::string_view p_name, const ref<Image2D>& p_image_2d) noexcept -> void
{
    KB_CORE_ASSERT(false, "[vulkan_descriptor_set_manager]: Not implemented!");
}

auto vulkan_descriptor_set_manager::is_invalidated(u32 p_set, u32 p_binding) const noexcept -> bool
{
    KB_CORE_ASSERT(false, "[vulkan_descriptor_set_manager]: Not implemented!");
    return false;
}

auto vulkan_descriptor_set_manager::validate() noexcept -> bool
{
    KB_CORE_ASSERT(false, "[vulkan_descriptor_set_manager]: Not implemented!");
    return false;
}

auto vulkan_descriptor_set_manager::bake() noexcept -> void
{
    KB_CORE_ASSERT(false, "[vulkan_descriptor_set_manager]: Not implemented!");
}

auto vulkan_descriptor_set_manager::has_buffer_sets() noexcept -> std::set<u32>
{
    KB_CORE_ASSERT(false, "[vulkan_descriptor_set_manager]: Not implemented!");
    return {};
}

auto vulkan_descriptor_set_manager::invalidate_and_update() noexcept -> void
{
    KB_CORE_ASSERT(false, "[vulkan_descriptor_set_manager]: Not implemented!");
}

auto vulkan_descriptor_set_manager::has_descriptor_sets() const noexcept -> bool
{
    KB_CORE_ASSERT(false, "[vulkan_descriptor_set_manager]: Not implemented!");
    return false;
}

auto vulkan_descriptor_set_manager::get_first_set_index() const noexcept -> u32
{
    KB_CORE_ASSERT(false, "[vulkan_descriptor_set_manager]: Not implemented!");
    return 0;
}

auto vulkan_descriptor_set_manager::get_descriptor_sets() const noexcept -> const std::vector<VkDescriptorSet>&
{
    KB_CORE_ASSERT(false, "[vulkan_descriptor_set_manager]: Not implemented!");
    return m_descriptor_sets[m_specification.m_start_set];
}

auto vulkan_descriptor_set_manager::is_input_valid(std::string_view p_name) const noexcept -> bool
{
    KB_CORE_ASSERT(false, "[vulkan_descriptor_set_manager]: Not implemented!");
    return false;
}

auto vulkan_descriptor_set_manager::get_input_declaration(
    std::string_view p_name
) const noexcept -> const render_pass_input_declaration*
{
    return &m_input_declarations.at(std::string{ p_name });
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
