#include "Kablunk/Core/Core.h"
#include "Kablunk/renderer/virtual_texture_registry.h"

#include <catch_amalgamated.hpp>

#include "Kablunk/renderer/Renderer.h"

using namespace kb;

auto init_renderer_singleton() -> void
{
    static bool initialized = false;
    if (!initialized)
    {
        Singleton<render::Renderer>::get().init();
        initialized = true;
    }
}

TEST_CASE("virtual texture registry initialization succeeds", "[virtual_texture_registry]")
{
    init_renderer_singleton();
    const auto virtual_texture_registry = render::virtual_texture_registry::create();

    const auto debug_stats = virtual_texture_registry->get_debug_statistics();
    // check missing texture loaded correctly
    REQUIRE(debug_stats.m_total_raw_textures == 1);
    REQUIRE(debug_stats.m_total_virtual_textures == 1);
}

TEST_CASE("virtual texture serialization", "[virtual_texture_registry]")
{
    init_renderer_singleton();

    // File name without extension
    std::filesystem::path expected_file_name{
        render::virtual_texture_registry_serializer::k_registry_cache_filename
    };

    SECTION("json serialization succeeds")
    {
        // clean up before test if an assertion went off and cache file was left
        expected_file_name += ".json";
        const auto expected_cache_file_path = fmt::format(
            "resources/{}",
            expected_file_name.string()
        );

        if (std::filesystem::exists(expected_cache_file_path))
        {
            std::filesystem::remove(expected_cache_file_path);
        }

        const auto virtual_texture_registry = render::virtual_texture_registry::create();

        virtual_texture_registry->save(
            render::virtual_texture_registry_serializer::serialization_type_t::json,
            "resources/"
        );

        REQUIRE(std::filesystem::exists(expected_cache_file_path));

        // clean up after test
        if (std::filesystem::exists(expected_cache_file_path))
        {
            std::filesystem::remove(expected_cache_file_path);
        }
    }

    SECTION("yaml serialization succeeds")
    {
        // clean up before test if an assertion went off and cache file was left
        expected_file_name += ".yaml";
        const auto expected_cache_file_path = fmt::format(
            "resources/{}",
            expected_file_name.string()
        );

        if (std::filesystem::exists(expected_cache_file_path))
        {
            std::filesystem::remove(expected_cache_file_path);
        }

        const auto virtual_texture_registry = render::virtual_texture_registry::create();

        virtual_texture_registry->save(
            render::virtual_texture_registry_serializer::serialization_type_t::yaml,
            "resources/"
        );

        REQUIRE(std::filesystem::exists(expected_cache_file_path));

        // clean up after test
        if (std::filesystem::exists(expected_cache_file_path))
        {
            std::filesystem::remove(expected_cache_file_path);
        }
    }

    SECTION("yaml deserialization succeeds")
    {
        // clean up before test if an assertion went off and cache file was left
        expected_file_name += ".yaml";
        const auto expected_cache_file_path = fmt::format(
            "resources/tmp/{}",
            expected_file_name.string()
        );

        if (std::filesystem::exists(expected_cache_file_path))
        {
            std::filesystem::remove(expected_cache_file_path);
        }

        {
            const auto tmp_registry_to_save = render::virtual_texture_registry::create();
            tmp_registry_to_save->save(
                render::virtual_texture_registry_serializer::serialization_type_t::yaml,
                "resources/tmp"
            );
        }

        const auto virtual_texture_registry = render::virtual_texture_registry::load(
            render::virtual_texture_registry_serializer::serialization_type_t::yaml,
            "resources/tmp"
        );

        REQUIRE(std::filesystem::exists(expected_cache_file_path));

        // clean up after test
        if (std::filesystem::exists(expected_cache_file_path))
        {
            std::filesystem::remove(expected_cache_file_path);
        }
    }
}

TEST_CASE("virtual texture registry loads raw texture and creates virtual texture", "[virtual_texture_registry]")
{
    init_renderer_singleton();

    const auto virtual_texture_registry = render::virtual_texture_registry::create();

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




