#include "kablunkpch.h"

#include "Kablunk/Asset/registry/virtual_texture_registry.h"
#include "Kablunk/serialize/kb-json/json_util.h"

#include <rapidjson/rapidjson.h>
#include <rapidjson/stream.h>
#include <rapidjson/document.h>
#include <rapidjson/filewritestream.h>
#include <rapidjson/writer.h>

#include <charconv>

namespace kb::asset
{ // start namespace kb::asset


auto virtual_texture_registry::create(
    const virtual_texture_registry_specification& p_specification
) -> std::unique_ptr<virtual_texture_registry>
{
    auto virtual_texture_manager = std::make_unique<virtual_texture_registry>();

    constexpr auto internal_memory_allocated = sizeof(virtual_texture_registry);
    virtual_texture_manager->m_debug_statistics.m_internal_memory_allocated += internal_memory_allocated;

    virtual_texture_manager->import_missing_texture();

    if (p_specification.m_initialize_on_construct)
    {
        virtual_texture_manager->load(
            p_specification.m_cache_path,
            p_specification.m_serialization_type
        );
    }

    return std::move(virtual_texture_manager);
}

auto virtual_texture_registry::import(
    const virtual_texture_specification& p_specification
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
        KB_CORE_ASSERT(false, "[virutal_texture_asset_manager]: 'memory' is not implemented yet!");
    }
    case texture_registry_import_type_t::none:
        KB_CORE_ASSERT(false, "[virtual_texture_asset_manager]: 'none' is not a valid texture registry import type!");
    default:
        KB_CORE_ASSERT(false, "[virtual_texture_asset_manager]: Unhandled texture registry import type!");
    }

    return create_or_get_virtual_texture(p_specification, texture_handle);
}

auto virtual_texture_registry::process() noexcept -> void
{
}

auto virtual_texture_registry::save(
    const std::filesystem::path& p_cache_dir,
    const serialization_type_t p_serialization_type /* = serialization_type_t::json */
) const noexcept -> void
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

    const auto cache_file_path = p_cache_dir / k_registry_cache_filename;

    switch (p_serialization_type)
    {
    case serialization_type_t::json:
    {
        serialize_registry_json_data_to_disk(cache_file_path, k_registry_cache_json_latest_version);
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
    case serialization_type_t::none:
    {
        KB_CORE_ASSERT(
            false,
            "[virtual_texture_asset_registry]: Unhandled serialization type!"
        );
        break;
    }
    }
}

auto virtual_texture_registry::load(
    const std::filesystem::path& p_cache_dir,
    serialization_type_t p_serialization_type
) noexcept -> void
{
    // ensure directory exists and 
    const auto virtual_texture_registry_cache_file_path = p_cache_dir / k_registry_cache_filename;
    if (!std::filesystem::exists(virtual_texture_registry_cache_file_path))
    {
        std::filesystem::create_directories(p_cache_dir);
        KB_CORE_WARN(
            "[virtual_texture_asset_registry]: Cache data could not be found at '{}'",
            virtual_texture_registry_cache_file_path.string()
        );
        return;
    }

    // run specific deserialization handler
    switch (p_serialization_type)
    {
    case serialization_type_t::json:
    {
        deserialize_registry_data_from_json(virtual_texture_registry_cache_file_path);
    }
    case serialization_type_t::bin:
    {
        KB_CORE_ASSERT(
            false,
            "[virtual_texture_asset_registry]: Binary deserialization of registry cache data not implemented!"
        );
        break;
    }
    case serialization_type_t::none:
    {
        KB_CORE_ASSERT(
            false,
            "[virtual_texture_asset_registry]: Unhandled serialization type!"
        );
        break;
    }
    }
}

auto virtual_texture_registry::get_texture_2d_by_raw_handle(
    const raw_texture_handle p_handle
) const noexcept -> const arc<Texture2D>&
{
    return m_raw_textures.contains(p_handle) ?
        m_raw_textures.at(p_handle) :
        m_missing_texture_data.m_raw_texture;
}

auto virtual_texture_registry::get_texture_2d_by_virtual_handle(
    const virtual_texture_handle p_handle
) const noexcept -> const arc<Texture2D>&
{
    return m_virtual_to_raw_handle_map.contains(p_handle) ?
        get_texture_2d_by_raw_handle(m_virtual_to_raw_handle_map.at(p_handle)) :
        m_missing_texture_data.m_raw_texture;
}

auto virtual_texture_registry::get_virtual_texture(
    const virtual_texture_handle p_handle
) const noexcept -> const render::virtual_texture&
{
    return m_virtual_textures.contains(p_handle) ?
        m_virtual_textures.at(p_handle) :
        m_missing_texture_data.m_virtual_texture;
}

auto virtual_texture_registry::get_debug_statistics() const noexcept -> debug_statistics
{
    const auto virtual_texture_mem_alloc = m_virtual_textures.size() * sizeof(render::virtual_texture);
    const auto total_mem_alloc = m_debug_statistics.m_internal_memory_allocated +
        m_debug_statistics.m_internal_memory_allocated +
        m_debug_statistics.m_raw_texture_memory_allocated +
        virtual_texture_mem_alloc;
    return debug_statistics{
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
}

auto virtual_texture_registry::import_texture_from_disk(
    const virtual_texture_specification& p_specification
) noexcept -> raw_texture_handle
{
    // path str moved into `texture_metadata`
    auto path_str = p_specification.m_path.string();
    const auto new_texture_handle = raw_texture_handle::into(std::string_view{ path_str });

    if (m_raw_textures.contains(new_texture_handle))
    {
        return new_texture_handle;
    }

    m_raw_textures.emplace(
        new_texture_handle,
        Texture2D::Create(path_str)
    );

    m_texture_metadata_map.emplace(
        new_texture_handle,
        texture_metadata{
            .m_path = std::move(path_str)
        }
    );

    return new_texture_handle;
}

auto virtual_texture_registry::import_missing_texture() noexcept -> void
{
    const std::filesystem::path missing_texture_path{ k_missing_texture_file_path };
    KB_CORE_ASSERT(
        std::filesystem::exists(missing_texture_path), "[virtual_texture_asset_registry]: Can not find missing texture '{}'",
        k_missing_texture_file_path
    );

    // moved into `texture_metadata`
    auto path_str = missing_texture_path.string();

    const auto missing_texture_raw_handle = raw_texture_handle::into(std::string_view{ path_str });
    constexpr auto missing_texture_virtual_handle = virtual_texture_handle::into(
        std::string_view{ k_missing_texture_krn_cstr }
    );

    m_missing_texture_data = {
        .m_raw_texture = Texture2D::Create(path_str),
        .m_virtual_texture = render::virtual_texture{
            .m_handle = missing_texture_virtual_handle,
            .m_uvs = {
                vec2_packed{ 0.f, 0.f },
                vec2_packed{ 0.f, 1.f },
                vec2_packed{ 1.f, 1.f },
                vec2_packed{ 1.f, 0.f },
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
        texture_metadata{
            .m_path = std::move(path_str),
        }
    );

    const auto texture_memory_allocated = m_missing_texture_data.m_raw_texture->GetWidth() *
        m_missing_texture_data.m_raw_texture->GetHeight() * 4ul * 4ul;
    m_debug_statistics.m_raw_texture_memory_allocated += texture_memory_allocated;
    m_debug_statistics.m_internal_memory_allocated += m_texture_metadata_map.at(missing_texture_raw_handle).get_allocated_bytes();
}

auto virtual_texture_registry::create_or_get_virtual_texture(
    const virtual_texture_specification& p_specification,
    const raw_texture_handle p_raw_texture_handle
) noexcept -> virtual_texture_handle
{
    const auto virtual_texture_opt = find_virtual_texture_by_raw_handle(p_raw_texture_handle);
    if (virtual_texture_opt)
    {
        return virtual_texture_opt->m_handle;
    }

    switch (p_specification.m_raw_texture_asset_type)
    {
    case raw_texture_asset_type_t::texture_2d:
    {
        const auto file_name = p_specification.m_path.filename().stem().string();
        const auto virtual_texture_krn = fmt::format("kb::texture::{}", file_name);
        const auto virtual_handle = virtual_texture_handle::into(std::string_view{ virtual_texture_krn });

        m_virtual_to_raw_handle_map.emplace(
            virtual_handle,
            p_raw_texture_handle
        );

        m_virtual_textures.emplace(
            virtual_handle,
            render::virtual_texture{
                .m_handle = virtual_handle,
                .m_uvs = {
                    vec2_packed{ 0.f, 0.f },
                    vec2_packed{ 0.f, 1.f },
                    vec2_packed{ 1.f, 1.f },
                    vec2_packed{ 1.f, 0.f },
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
        KB_CORE_ASSERT(false, "[virtual_texture_asset_registry]: 'None' is not a valid raw texture type!");
    default:
        KB_CORE_ASSERT(false, "[virtual_texture_asset_registry]: Unhandled raw texture type!");
    }

    KB_CORE_ASSERT(false, "should be unreachable!");
    return m_missing_texture_data.m_virtual_texture.m_handle;
}

auto virtual_texture_registry::serialize_registry_json_data_to_disk(
    const std::filesystem::path& p_cache_registry_file_path,
    u32 p_version
) const noexcept -> void
{
    switch (p_version)
    {
    case 1:
        serialize_registry_json_data_version_1_to_disk(p_cache_registry_file_path);
        break;
    default:
        KB_CORE_ASSERT(
            false,
            "[virtual_texture_asset_registry]: Cache json document version '{}' serialization not implemented!",
            p_version
        );
    }
}

// TODO: move to serializer class
auto virtual_texture_registry::serialize_registry_json_data_version_1_to_disk(
    const std::filesystem::path& p_cache_registry_file_path
) const noexcept -> void
{
    constexpr u32 k_version_1 = 1;

    rapidjson::Document cache_document{};
    cache_document.SetObject();

    // write json data to rapidjson document
    {
        // store cache document type
        util::document_set_value(
            cache_document,
            "document_type",
            std::string_view{ k_registry_cache_document_type }
        );

        // store cache document version
        util::document_set_value(
            cache_document,
            "version",
            k_version_1
        );

        // store raw texture metadata
        {
            rapidjson::Value raw_textures{ rapidjson::kArrayType };
            for (const auto& [raw_handle, raw_metadata] : m_texture_metadata_map)
            {
                // skip missing texture as engine reloads automatically
                // TODO: evaluate if we should just store instead...
                if (raw_handle == m_missing_texture_data.m_raw_texture_handle)
                {
                    continue;
                }

                auto raw_handle_str = fmt::format("{}", raw_handle.as<u32>());
                raw_textures.PushBack(
                    rapidjson::Value{ rapidjson::kObjectType }.AddMember(
                        "raw_handle",
                        rapidjson::StringRef(raw_handle_str.c_str()),
                        cache_document.GetAllocator()
                    ),
                    cache_document.GetAllocator()
                );

                raw_textures.PushBack(
                    rapidjson::Value{ rapidjson::kObjectType }.AddMember(
                        "texture_path",
                        rapidjson::StringRef(raw_metadata.m_path.c_str()),
                        cache_document.GetAllocator()
                    ),
                    cache_document.GetAllocator()
                );
            }
            cache_document.AddMember(
                "raw_textures",
                raw_textures,
                cache_document.GetAllocator()
            );
        }

        // store virtual texture data
        {
            rapidjson::Value virtual_to_raw_handles{ rapidjson::kArrayType };
            for (const auto& [virtual_handle, virtual_texture_data] : m_virtual_textures)
            {
                // skip missing texture as engine reloads automatically
                // TODO: evaluate if we should just store instead...
                if (virtual_handle == m_missing_texture_data.m_virtual_texture.m_handle)
                {
                    continue;
                }

                rapidjson::Value virtual_texture_value{ rapidjson::kObjectType };

                auto raw_handle_str = fmt::format("{}", virtual_handle.as<u32>());
                virtual_texture_value.AddMember(
                    "virtual_handle",
                    rapidjson::StringRef(raw_handle_str.c_str()),
                    cache_document.GetAllocator()
                );

                rapidjson::Value uvs_value{ rapidjson::kArrayType };
                for (const auto& uv : virtual_texture_data.m_uvs)
                {
                    rapidjson::Value uv_value{ rapidjson::kObjectType };
                    uv_value
                        .AddMember("x", uv.m_storage.m_data[0], cache_document.GetAllocator())
                        .AddMember("y", uv.m_storage.m_data[1], cache_document.GetAllocator());

                    uvs_value.PushBack(uv_value, cache_document.GetAllocator());
                }

                virtual_texture_value.AddMember(
                    "uvs",
                    uvs_value,
                    cache_document.GetAllocator()
                );

                virtual_to_raw_handles.PushBack(
                    virtual_texture_value,
                    cache_document.GetAllocator()
                );
            }

            cache_document.AddMember(
                "virtual_textures",
                virtual_to_raw_handles,
                cache_document.GetAllocator()
            );
        }

        // store virtual to raw handle map
        {
            rapidjson::Value virtual_to_raw_handles{ rapidjson::kArrayType };
            for (const auto& [virtual_handle, raw_handle] : m_virtual_to_raw_handle_map)
            {
                // skip missing texture as engine reloads automatically
                // TODO: evaluate if we should just store instead...
                if (virtual_handle == m_missing_texture_data.m_virtual_texture.m_handle)
                {
                    continue;
                }

                const auto virtual_handle_str = fmt::format("{}", virtual_handle.as<u32>());
                const auto raw_handle_str = fmt::format("{}", raw_handle.as<u32>());
                virtual_to_raw_handles.PushBack(
                    rapidjson::Value{ rapidjson::kObjectType }.AddMember(
                        rapidjson::StringRef(virtual_handle_str.c_str()),
                        rapidjson::StringRef(raw_handle_str.c_str()),
                        cache_document.GetAllocator()
                    ),
                    cache_document.GetAllocator()
                );
            }

            cache_document.AddMember(
                "virtual_to_raw_handles",
                virtual_to_raw_handles,
                cache_document.GetAllocator()
            );
        }
    }

    // save json to file
    {
        const auto path_str = p_cache_registry_file_path.string();
#ifdef KB_PLATFORM_WINDOWS
        constexpr const char* k_file_write_mode = "wb";
#else
        constexpr const char* k_file_write_mode = "w";
#endif

        // apparently c file api is more performant than c++ streams
        // according to rapidjson docs: https://rapidjson.org/md_doc_stream.html
        FILE* fp = fopen(path_str.c_str(), k_file_write_mode);
        KB_CORE_ASSERT(fp, "[virtual_texture_registry]: Failed to open cache document file '{}'", path_str);

        char write_buffer[65536];
        rapidjson::FileWriteStream output_stream{ fp, write_buffer, sizeof(write_buffer) };

        rapidjson::Writer writer{ output_stream };
        cache_document.Accept(writer);

        auto ret = fclose(fp);
        if (ret > 0)
        {
            KB_CORE_ERROR("[virtual_texture_registry]: Failed to close cache document file pointer");
        }
    }
}

// TODO: move to serializer class
auto virtual_texture_registry::deserialize_registry_data_from_json(
    const std::filesystem::path& p_cache_json_file_path) noexcept -> void
{
    const auto path_str = p_cache_json_file_path.string();
    KB_CORE_INFO("[virtual_texture_asset_registry]: Deserializing json cache document from '{}'", path_str);

    constexpr const char* k_file_read_mode = "r";

    std::ifstream cache_file_stream{ p_cache_json_file_path, std::ios::in };
    if (!cache_file_stream)
    {
        KB_CORE_ASSERT(
            false,
            "[virtual_texture_registry]: Failed to open cache document '{}'!",
            path_str.c_str()
        );
        return;
    }

    std::string cache_file_buffer;
    cache_file_stream.seekg(0, std::ios::end);
    cache_file_buffer.resize(cache_file_stream.tellg());
    cache_file_stream.seekg(0, std::ios::beg);
    cache_file_stream.read(cache_file_buffer.data(), static_cast<i64>(cache_file_buffer.size()));

    rapidjson::Document cache_document{};
    cache_document.Parse(cache_file_buffer.c_str());

    const auto cache_document_type = util::get_value_from_document<std::string_view>(cache_document, "document_type");

    if (!cache_document_type || cache_document_type != std::string_view{ k_registry_cache_document_type })
    {
        KB_CORE_ERROR(
            "[virtual_texture_asset_registry]: Document type could not be parsed or is not '{}'!",
            k_registry_cache_document_type
        );
        return;
    }

    const auto cache_document_version = util::get_value_from_document<u32>(cache_document, "version");

    if (!cache_document_version)
    {
        KB_CORE_ERROR(
            "[virutal_texture_asset_registry]: Document version could not be parsed!"
        );
    }

    switch (*cache_document_version)
    {
    case 1:
        deserialize_registry_data_from_json_version_1(cache_document);
        break;
    default:
        KB_CORE_ERROR("[virtual_texture_asset_registry]: Unhandled cache document version '{}'", *cache_document_version);
        break;
    }

    if (*cache_document_version != k_registry_cache_json_latest_version)
    {
        KB_CORE_ASSERT(false, "virtual texture registry cache version conversion not implemented!");
    }
}

/*
 * cache document version 1 schema
 * {
 *   "document_type": "kb::virtual_texture_registry",
 *   "version": int,
 *   "raw_textures": [
 *     {
 *       "raw_handle": string,
 *       "texture_path": string
 *     }
 *   ],
 *   "virtual_textures": [
 *     {
 *       "virtual_handle": string,
 *       "uvs": [
 *          { x: int, y: int }, // x0, y0
 *          { x: int, y: int }, // x0, y1
 *          { x: int, y: int }, // x1, y1
 *          { x: int, y: int }  // x1, y0
 *       ]
 *     }
 *   ],
 *   "virtual_to_raw_handles":[
 *     {
 *       "virtual_handle": string,
 *       "raw_handle": string
 *     }
 *   ]
 *   }
*/
// TODO: move to serializer class
auto virtual_texture_registry::deserialize_registry_data_from_json_version_1(
    rapidjson::Document& p_document
) noexcept -> void
{
    // load raw texture data
    {
        if (p_document["raw_textures"].IsArray())
        {
            const auto raw_textures_data = p_document["raw_textures"].GetArray();
            for (auto&& texture_metadata : raw_textures_data)
            {
                // validation
                if (!texture_metadata["raw_handle"].IsString())
                {
                    KB_CORE_ERROR(
                        "[virtual_texture_registry]: Failed to read raw texture handle from raw texture metadata!"
                    );
                    continue;
                }

                if (!texture_metadata["texture_path"].IsString())
                {
                    KB_CORE_ERROR("[virtual_texture_registry]: Failed to read raw texture path from raw texture metadata!");
                    continue;
                }

                const auto raw_handle_json = std::string_view{ texture_metadata["raw_handle"].GetString() };
                const auto texture_path_cstr = texture_metadata["texture_path"].GetString();
                // non-const so we can move into `texture_metadata`
                auto texture_path = std::string{ texture_path_cstr };

                // assumes that raw handle is hashed version, not a krn
                // #TODO handle krn or hashed version
                u32 raw_handle_val;
                auto [_, ec] = std::from_chars(
                    raw_handle_json.data(),
                    raw_handle_json.data() + raw_handle_json.size(),
                    raw_handle_val
                );
                if (ec == std::errc{})
                {
                    KB_CORE_ERROR("[virtual_texture_registry]: raw handle value '{}' is malformed!", raw_handle_json);
                    continue;
                }

                const auto raw_handle = raw_texture_handle{ raw_handle_val };

                KB_CORE_TRACE(
                    "[virtual_texture_registry]: Read raw texture data raw_handle={}, texture_path={}",
                    static_cast<raw_texture_handle::value_t>(raw_handle),
                    texture_path_cstr
                );

                m_texture_metadata_map.emplace(
                    raw_handle,
                    asset::texture_metadata{
                        .m_path = std::move(texture_path),
                    }
                );
            }
        }
        else
        {
            KB_CORE_ERROR("[virtual_texture_registry]: Failed to read raw textures array data!");
        }
    }

    // load virtual texture data
    {
        const auto virtual_texture_json_data = p_document["raw_textures"].GetArray();
        for (auto&& virtual_texture_json : virtual_texture_json_data)
        {
            if (!virtual_texture_json["virtual_handle"].IsString())
            {
                KB_CORE_ERROR(
                    "[virtual_texture_registry]: Failed to read virtual handle from virtual texture metadata!"
                );
                continue;
            }

            if (!virtual_texture_json["uvs"].IsArray())
            {
                KB_CORE_ERROR(
                    "[virutal_texture_registry]: Failed to read uv array data from virtual texture metadata!"
                );
                continue;
            }

            auto uvs_json = virtual_texture_json["uvs"].GetArray();
            if (uvs_json.Size() != 4)
            {
                KB_CORE_ERROR(
                    "[virtual_texture_registry]: Expected 4 values in virtual texture uvs array, found {} instead?",
                    uvs_json.Size()
                );
                continue;
            }

            const auto virtual_handle_json = std::string_view{ virtual_texture_json["virtual_handle"].GetString() };

            // assumes that raw handle is hashed version, not a krn
                // #TODO handle krn or hashed version
            u32 virtual_handle_hashed_val;
            {
                auto [_, ec] = std::from_chars(
                    virtual_handle_json.data(),
                    virtual_handle_json.data() + virtual_handle_json.size(),
                    virtual_handle_hashed_val
                );
                if (ec == std::errc{})
                {
                    KB_CORE_ERROR("[virtual_texture_registry]: virtual handle value '{}' is malformed!", virtual_handle_json);
                    continue;
                }
            }

            const auto virtual_handle = virtual_texture_handle{ virtual_handle_hashed_val };

            std::array<vec2_packed, 4> uvs{};
            bool invalid_uv_data = false;

            size_t uv_index = 0;
            for (auto&& uv_json : uvs_json)
            {
                if (!uv_json["x"].IsNumber())
                {
                    KB_CORE_ERROR(
                        "[virtual_texture_registry]: Could not read uv x value!"
                    );
                    invalid_uv_data = true;
                    break;
                }

                if (!uv_json["y"].IsNumber())
                {
                    KB_CORE_ERROR(
                        "[virtual_texture_registry]: Could not read uv y value!"
                    );
                    invalid_uv_data = true;
                    break;
                }

                const auto x_json = std::string_view{ uv_json["x"].GetString() };
                f32 x_val;
                {
                    auto [_, uv_ec] = std::from_chars(
                        x_json.data(),
                        x_json.data() + x_json.size(),
                        x_val
                    );
                    if (uv_ec == std::errc{})
                    {
                        KB_CORE_ERROR("[virtual_texture_registry]: uv x value '{}' is malformed!", x_json);
                        invalid_uv_data = true;
                        continue;
                    }
                }

                const auto y_json = std::string_view{ uv_json["y"].GetString() };
                f32 y_val;
                {
                    auto [_, uv_ec] = std::from_chars(
                        y_json.data(),
                        y_json.data() + y_json.size(),
                        y_val
                    );
                    if (uv_ec == std::errc{})
                    {
                        KB_CORE_ERROR("[virtual_texture_registry]: uv y value '{}' is malformed!", y_val);
                        invalid_uv_data = true;
                        continue;
                    }
                }

                uvs[uv_index++] = vec2_packed{ x_val, y_val };
            }

            if (invalid_uv_data)
                continue;

            // add virtual texture data to map
            m_virtual_textures.emplace(
                virtual_handle,
                render::virtual_texture{
                    .m_handle = virtual_handle,
                    .m_uvs = uvs
                }
            );
        }
    }

    // load virtual to raw handle map
    {
        const auto raw_handles_json_data = p_document["virtual_to_raw_handles"].GetArray();
        for (auto&& virtual_to_raw_json_data : raw_handles_json_data)
        {
            // validate virtual handle json
            if (!virtual_to_raw_json_data["virtual_handle"].IsString())
            {
                KB_CORE_ERROR(
                    "[virtual_texture_registry]: Could not read virtual handle in virtual to raw texture handles!"
                );
                return;
            }

            if (!virtual_to_raw_json_data["raw_handle"].IsString())
            {
                KB_CORE_ERROR(
                    "[virutal_texture_registry]: Could not read rwa handle in virtual to raw texture handles!"
                );
                return;
            }

            // TODO: store virtual to raw map

            const auto virtual_handle_json = std::string_view{
                virtual_to_raw_json_data["virtual_handle"].GetString()
            };

            // assumes that virtual handle is hashed version, not a krn
                // #TODO handle krn or hashed version
            u32 virtual_handle_hashed_val;
            {
                auto [_, ec] = std::from_chars(
                    virtual_handle_json.data(),
                    virtual_handle_json.data() + virtual_handle_json.size(),
                    virtual_handle_hashed_val
                );
                if (ec == std::errc{})
                {
                    KB_CORE_ERROR("[virtual_texture_registry]: virtual handle value '{}' is malformed!", virtual_handle_json);
                    continue;
                }
            }

            const auto raw_handle_json = std::string_view{
                virtual_to_raw_json_data["raw_handle"].GetString()
            };

            // assumes that raw handle is hashed version, not a krn
                // #TODO handle krn or hashed version
            u32 raw_handle_hashed_val;
            {
                auto [_, ec] = std::from_chars(
                    raw_handle_json.data(),
                    raw_handle_json.data() + raw_handle_json.size(),
                    raw_handle_hashed_val
                );
                if (ec == std::errc{})
                {
                    KB_CORE_ERROR("[virtual_texture_registry]: raw handle value '{}' is malformed!", raw_handle_json);
                    continue;
                }
            }

            // store virtual to raw mapping
            m_virtual_to_raw_handle_map.emplace(
                virtual_texture_handle{ virtual_handle_hashed_val },
                raw_texture_handle{ raw_handle_hashed_val }
            );
        }
    }
}

} // end namespace kb::asset
