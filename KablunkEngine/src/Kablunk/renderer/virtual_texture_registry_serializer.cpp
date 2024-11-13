#include "kablunkpch.h"

#include "Kablunk/renderer/virtual_texture_registry_serializer.h"
#include "Kablunk/renderer/virtual_texture_registry.h"

#include "Kablunk/serialize/renderer/virtual_texture_registry_json_serializer.h"
#include "Kablunk/serialize/renderer/virtual_texture_registry_yaml_serializer.h"
#include "Kablunk/serialize/kb-json/json_util.h"

namespace kb::render
{ // start namespace kb::render

auto virtual_texture_registry_serializer::serialize(
    serialization_type_t p_serialization_type,
    const std::filesystem::path& p_cache_dir,
    const weak_ptr<virtual_texture_registry> p_virtual_texture_registry
) noexcept -> void
{
    // TODO: ensure the is_directory returns true for directories that do not exist
    KB_CORE_ASSERT(
        std::filesystem::is_directory(p_cache_dir),
        "[virtual_texture_registry]: save() expects a directory!"
    );

    if (!std::filesystem::exists(p_cache_dir))
    {
        std::filesystem::create_directories(p_cache_dir);
        KB_CORE_INFO(
            "[virtual_texture_asset_registry]: Creating directory path '{}'",
            p_cache_dir.string()
        );
    }

    auto cache_file_path = p_cache_dir / k_registry_cache_filename;

    switch (p_serialization_type)
    {
    case serialization_type_t::yaml:
    {
        cache_file_path += ".yaml";
        serialize::virtual_texture_registry_yaml_serializer::serialize(
            std::move(cache_file_path),
            p_virtual_texture_registry
        );
        break;
    }
    case serialization_type_t::json:
    {
        cache_file_path += ".json";
        serialize::virtual_texture_registry_json_serializer::serialize(
            std::move(cache_file_path),
            p_virtual_texture_registry
        );
        break;
    }
    case serialization_type_t::bin:
    {
        KB_CORE_ASSERT(
            false,
            "[virtual_texture_asset_registry]: Binary deserialization of registry cache data not implemented!"
        );
        break;
    }
    default:
    {
        KB_CORE_ASSERT(
            false,
            "[virtual_texture_asset_registry]: Unhandled serialization type!"
        );
        break;
    }
    }
}

auto virtual_texture_registry_serializer::deserialize(
    serialization_type_t p_serialization_type,
    const std::filesystem::path& p_cache_file_dir
) noexcept -> std::unique_ptr<virtual_texture_registry>
{
    // ensure directory exists and 
    if (!std::filesystem::exists(p_cache_file_dir))
    {
        std::filesystem::create_directories(p_cache_file_dir);
        KB_CORE_WARN(
            "[virtual_texture_registry_serializer]: Cache data could not be found at '{}'",
            p_cache_file_dir.string()
        );

#ifdef KB_DEBUG
        KB_CORE_ASSERT(false, "[virtual_texture_registry_serializer]: Cache data could not be loaded!");
#endif

        return virtual_texture_registry::create();
    }

    auto cached_file_name = p_cache_file_dir / k_registry_cache_filename;

    // run specific deserialization handler
    switch (p_serialization_type)
    {
    case serialization_type_t::yaml:
    {
        cached_file_name += ".yaml";
        return serialize::virtual_texture_registry_yaml_serializer::deserialize(
            std::move(cached_file_name)
        );
    }
    case serialization_type_t::json:
    {
        cached_file_name += ".json";
        return serialize::virtual_texture_registry_json_serializer::deserialize(
            std::move(cached_file_name)
        );
    }
    case serialization_type_t::bin:
    {
        KB_CORE_ASSERT(
            false,
            "[virtual_texture_registry_serializer]: Binary deserialization of registry cache data not implemented!"
        );
        return virtual_texture_registry::create();
    }
    default:
    {
        KB_CORE_ASSERT(
            false,
            "[virtual_texture_registry_serializer]: Unhandled serialization type!"
        );
        return virtual_texture_registry::create();
    }
    }
}

} // end namespace kb::render
