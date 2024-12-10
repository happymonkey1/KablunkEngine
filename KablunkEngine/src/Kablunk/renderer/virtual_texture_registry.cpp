#include "kablunkpch.h"

#include "Kablunk/renderer/virtual_texture_registry.h"
#include "Kablunk/serialize/kb-json/json_util.h"

#include <charconv>

namespace kb::render
{ // start namespace kb::render

auto virtual_texture_registry::create() noexcept -> std::unique_ptr<virtual_texture_registry>
{
    auto virtual_texture_registry = std::make_unique<render::virtual_texture_registry>();

    constexpr auto internal_memory_allocated = sizeof(virtual_texture_registry);
    virtual_texture_registry->m_debug_statistics.m_internal_memory_allocated += internal_memory_allocated;

    virtual_texture_registry->import_missing_texture();

    return std::move(virtual_texture_registry);
}

auto virtual_texture_registry::load_texture(
    const virtual_texture_specification_t& p_specification
) noexcept -> virtual_texture_handle
{
    const bool should_check_exists = p_specification.m_import_type == texture_registry_import_type_t::disk;
    if (should_check_exists && !std::filesystem::exists(p_specification.m_path))
    {
        return m_missing_texture_data.m_virtual_texture.m_handle;
    }

    raw_texture_handle texture_handle{};
    switch (p_specification.m_import_type)
    {
    case texture_registry_import_type_t::disk:
    {
        texture_handle = import_texture_from_disk(p_specification);
        break;
    }
    case texture_registry_import_type_t::memory:
    {
        KB_CORE_ASSERT(false, "[virtual_texture_registry]: 'memory' is not implemented yet!");
    }
    case texture_registry_import_type_t::none:
        KB_CORE_ASSERT(false, "[virtual_texture_registry]: 'none' is not a valid texture registry import type!");
    default:
        KB_CORE_ASSERT(false, "[virtual_texture_registry]: Unhandled texture registry import type!");
    }

    return create_or_get_virtual_texture(p_specification, texture_handle);
}

auto virtual_texture_registry::load_individual_texture(
    std::filesystem::path p_texture_path
) noexcept -> virtual_texture_handle
{
    return load_texture(virtual_texture_specification_t{
        .m_path = std::move(p_texture_path),
        .m_import_type = texture_registry_import_type_t::disk,
        .m_raw_texture_asset_type = raw_texture_asset_type_t::texture_2d,
        .m_allow_to_be_packed = true,
        .m_texture_dimensions = {},
        .m_sprite_unpacker_dimensions = {}
    });
}

auto virtual_texture_registry::load_texture_atlas(std::filesystem::path p_atlas_path) noexcept -> virtual_texture_handle
{
    KB_CORE_ASSERT(false, "[virtual_texture_registry]: load_texture_atlas() is not implemented!");
    return virtual_texture_handle{ 0 };
}

// TODO: this could have collisions if there are two files in separate directories with the same name
auto virtual_texture_registry::create_virtual_texture_handle(
    const std::filesystem::path& p_file_path) noexcept -> virtual_texture_handle
{
    auto file_name = p_file_path.filename().stem().string();
    const auto virtual_texture_krn = fmt::format("kb::texture::{}", std::move(file_name));
    return virtual_texture_handle::into(std::string_view{ virtual_texture_krn });
}

auto virtual_texture_registry::process() noexcept -> void
{
    KB_CORE_ASSERT(false, "[virtual_texture_registry]: process() not implemented!");
}

auto virtual_texture_registry::save(
    const virtual_texture_registry_serializer::serialization_type_t p_serialization_type,
    const std::filesystem::path& p_cache_dir
) const noexcept -> void
{
    virtual_texture_registry_serializer::serialize(
        p_serialization_type,
        p_cache_dir,
        weak_ptr{ const_cast<virtual_texture_registry*>(this) }
    );
}

auto virtual_texture_registry::load(
    const virtual_texture_registry_serializer::serialization_type_t p_serialization_type,
    const std::filesystem::path& p_cache_dir
) noexcept -> std::unique_ptr<virtual_texture_registry>
{
    auto virtual_texture_registry = virtual_texture_registry_serializer::deserialize(
        p_serialization_type,
        p_cache_dir
    );

    return std::move(virtual_texture_registry);
}

auto virtual_texture_registry::get_texture_2d_by_raw_handle(
    const raw_texture_handle p_handle
) const noexcept -> const arc<backend::texture_2d>&
{
    return m_raw_textures.contains(p_handle) ?
        m_raw_textures.at(p_handle) :
        m_missing_texture_data.m_raw_texture;
}

auto virtual_texture_registry::get_texture_2d_by_virtual_handle(
    const virtual_texture_handle p_handle
) const noexcept -> const arc<backend::texture_2d>&
{
    return m_virtual_to_raw_handle_map.contains(p_handle) ?
        get_texture_2d_by_raw_handle(m_virtual_to_raw_handle_map.at(p_handle)) :
        m_missing_texture_data.m_raw_texture;
}

auto virtual_texture_registry::get_virtual_texture(
    const virtual_texture_handle p_handle
) const noexcept -> const render::virtual_texture_t&
{
    return m_virtual_textures.contains(p_handle) ?
        m_virtual_textures.at(p_handle) :
        m_missing_texture_data.m_virtual_texture;
}

auto virtual_texture_registry::get_debug_statistics() const noexcept -> debug_statistics_t
{
    const auto virtual_texture_mem_alloc = m_virtual_textures.size() * sizeof(render::virtual_texture_t);
    const auto total_mem_alloc = m_debug_statistics.m_internal_memory_allocated +
        m_debug_statistics.m_internal_memory_allocated +
        m_debug_statistics.m_raw_texture_memory_allocated +
        virtual_texture_mem_alloc;
    return debug_statistics_t{
        .m_total_memory_allocated = total_mem_alloc,
        .m_internal_memory_allocated = m_debug_statistics.m_internal_memory_allocated,
        .m_raw_texture_memory_allocated = virtual_texture_mem_alloc,
        .m_virtual_texture_memory_allocated = m_debug_statistics.m_virtual_texture_memory_allocated,
        .m_total_raw_textures = m_virtual_textures.size(),
        .m_total_virtual_textures = m_raw_textures.size(),
    };
}

auto virtual_texture_registry::create_texture_atlases() noexcept -> void
{
    KB_CORE_ASSERT(false, "[virtual_texture_registry]: create_texture_atlases() not implemented!");
}

auto virtual_texture_registry::import_texture_from_disk(
    const virtual_texture_specification_t& p_specification
) noexcept -> raw_texture_handle
{
    // path str moved into `texture_metadata`
    auto path_str = p_specification.m_path.string();
    const auto new_texture_handle = raw_texture_handle::into(std::string_view{ path_str });

    // Check if texture is already imported
    if (m_raw_textures.contains(new_texture_handle))
    {
#ifdef KB_DEBUG
        if (new_texture_handle != k_missing_texture_krn)
        {
            KB_CORE_WARN(
                "[virtual_texture_registry]: Loading texture '{}' with handle {} that is already contained in the registry?",
                p_specification.m_path.string(),
                new_texture_handle.as<u32>()
            );
        }
#endif

        return new_texture_handle;
    }

    m_raw_textures.emplace(
        new_texture_handle,
        backend::texture_2d::create(path_str)
    );

    m_texture_metadata_map.emplace(
        new_texture_handle,
        texture_metadata_t{
            .m_path = std::move(path_str),
            .m_is_atlas = p_specification.m_raw_texture_asset_type == raw_texture_asset_type_t::texture_atlas,
        }
    );

    return new_texture_handle;
}

auto virtual_texture_registry::import_missing_texture() noexcept -> void
{
    const std::filesystem::path missing_texture_path{ k_missing_texture_file_path };
    KB_CORE_ASSERT(
        std::filesystem::exists(missing_texture_path),
        "[virtual_texture_registry]: Can not find missing texture '{}'",
        k_missing_texture_file_path
    );

    // moved into `texture_metadata`
    auto path_str = missing_texture_path.string();

    const auto missing_texture_raw_handle = raw_texture_handle::into(std::string_view{ path_str });
    constexpr auto missing_texture_virtual_handle = virtual_texture_handle::into(
        std::string_view{ k_missing_texture_krn_cstr }
    );

    const auto raw_texture = backend::texture_2d::create(path_str);

    m_missing_texture_data = {
        .m_raw_texture = raw_texture,
        .m_virtual_texture = virtual_texture_t{
            .m_handle = missing_texture_virtual_handle,
            .m_uvs = {
                vec2_packed{ 0.f, 0.f },
                vec2_packed{ 0.f, 1.f },
                vec2_packed{ 1.f, 1.f },
                vec2_packed{ 1.f, 0.f },
            },
            .m_dimensions = uvec2_packed{
                raw_texture->get_width(),
                raw_texture->get_height()
            }
        },
        .m_raw_texture_handle = missing_texture_raw_handle,
    };

    m_virtual_textures.emplace(
        missing_texture_virtual_handle,
        m_missing_texture_data.m_virtual_texture
    );
    m_virtual_to_raw_handle_map.emplace(
        missing_texture_virtual_handle,
        missing_texture_raw_handle
    );
    m_raw_textures.emplace(
        missing_texture_raw_handle,
        m_missing_texture_data.m_raw_texture
    );
    m_texture_metadata_map.emplace(
        missing_texture_raw_handle,
        texture_metadata_t{
            .m_path = std::move(path_str),
            .m_is_atlas = false,
        }
    );

    const auto texture_memory_allocated = m_missing_texture_data.m_raw_texture->get_width() *
        m_missing_texture_data.m_raw_texture->get_height() * 4ul * 4ul;
    m_debug_statistics.m_raw_texture_memory_allocated += texture_memory_allocated;
    m_debug_statistics.m_internal_memory_allocated += m_texture_metadata_map.at(missing_texture_raw_handle).get_allocated_bytes();
}

auto virtual_texture_registry::create_or_get_virtual_texture(
    const virtual_texture_specification_t& p_specification,
    const raw_texture_handle p_raw_texture_handle
) noexcept -> virtual_texture_handle
{
    // TODO: this could have collisions if there are two files in separate directories with the same name
    if (const auto virtual_texture_opt = find_virtual_texture_by_raw_handle(p_raw_texture_handle); virtual_texture_opt.has_value())
    {
        return virtual_texture_opt->m_handle;
    }

    switch (p_specification.m_raw_texture_asset_type)
    {
    case raw_texture_asset_type_t::texture_2d:
    {
        const auto virtual_handle = create_virtual_texture_handle(p_specification.m_path);

        m_virtual_to_raw_handle_map.emplace(
            virtual_handle,
            p_raw_texture_handle
        );

        const auto& raw_texture = m_raw_textures[p_raw_texture_handle];

        m_virtual_textures.emplace(
            virtual_handle,
            virtual_texture_t{
                .m_handle = virtual_handle,
                .m_uvs = {
                    vec2_packed{ 0.f, 0.f },
                    vec2_packed{ 0.f, 1.f },
                    vec2_packed{ 1.f, 1.f },
                    vec2_packed{ 1.f, 0.f },
                },
                .m_dimensions = uvec2_packed{
                    raw_texture->get_width(),
                    raw_texture->get_height()
                }
            }
        );

        return virtual_handle;
    }
    case raw_texture_asset_type_t::texture_atlas:
    {
        // #TODO
        KB_CORE_ASSERT(false, "not implemented!");
        return m_missing_texture_data.m_virtual_texture.m_handle;
    }
    case raw_texture_asset_type_t::none:
        KB_CORE_ASSERT(false, "[virtual_texture_registry]: 'None' is not a valid raw texture type!");
    default:
        KB_CORE_ASSERT(false, "[virtual_texture_registry]: Unhandled raw texture type!");
    }

    KB_CORE_ASSERT(false, "should be unreachable!");
    return m_missing_texture_data.m_virtual_texture.m_handle;
}



} // end namespace kb::render
