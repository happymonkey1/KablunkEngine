#include "Kablunk/Core/Core.h"
#include "Kablunk/Asset/registry/virtual_texture_registry.h"

#include <catch_amalgamated.hpp>

using namespace kb;

TEST_CASE("virtual texture registry initialization succeeds", "[virtual_texture_registry]")
{
    const auto virtual_texture_registry = asset::virtual_texture_registry::create(asset::virtual_texture_registry_specification{
        .m_cache_path = "resources/",
        .m_initialize_on_construct = false,
        .m_serialization_type = asset::serialization_type_t::json
        }
    );

    const auto debug_stats = virtual_texture_registry->get_debug_statistics();
    // check missing texture loaded correctly
    REQUIRE(debug_stats.m_total_raw_textures == 1);
    REQUIRE(debug_stats.m_total_virtual_textures == 1);
}

TEST_CASE("virtual texture registry creates cache json", "[virtual_texture_registry]")
{
    // clean up before test if an assertion went off and cache file was left
    const auto expected_cache_file_path = fmt::format(
        "resources/{}",
        asset::virtual_texture_registry::k_registry_cache_filename
    );
    if (std::filesystem::exists(expected_cache_file_path))
    {
        std::filesystem::remove(expected_cache_file_path);
    }

    const auto virtual_texture_registry = asset::virtual_texture_registry::create(asset::virtual_texture_registry_specification{
        .m_cache_path = "resources/",
        .m_initialize_on_construct = false,
        .m_serialization_type = asset::serialization_type_t::json
        }
    );

    virtual_texture_registry->save("resources/");

    REQUIRE(std::filesystem::exists(expected_cache_file_path));

    // clean up after test
    if (std::filesystem::exists(expected_cache_file_path))
    {
        std::filesystem::remove(expected_cache_file_path);
    }
}

TEST_CASE("virtual texture registry loads raw texture and creates virtual texture", "[virtual_texture_registry]")
{
    const auto virtual_texture_registry =
        asset::virtual_texture_registry::create(asset::virtual_texture_registry_specification{
            .m_cache_path = "resources/",
            .m_initialize_on_construct = false,
            .m_serialization_type = asset::serialization_type_t::json
        }
    );

#if 0
    virtual_texture_registry->import({
        .m_path = "resources/",
        .m_import_type = ,
        .m_raw_texture_asset_type = ,
        .m_allow_to_be_packed = ,
        .m_texture_dimensions = ,
        .m_sprite_unpacker_dimensions =
    });
#endif

}




