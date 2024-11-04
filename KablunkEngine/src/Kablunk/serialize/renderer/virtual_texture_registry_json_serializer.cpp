#include "kablunkpch.h"

#include "Kablunk/serialize/renderer/virtual_texture_registry_json_serializer.h"

#include <rapidjson/filewritestream.h>
#include <rapidjson/writer.h>

#include "Kablunk/renderer/virtual_texture_registry_serializer.h"
#include "Kablunk/renderer/virtual_texture_registry.h"

#include "Kablunk/renderer/virtual_texture_registry_serializer.h"
#include "Kablunk/serialize/kb-json/json_util.h"

namespace kb::serialize
{ // start namespace kb::serialize

auto virtual_texture_registry_json_serializer::serialize(
    std::filesystem::path p_cache_file_path,
    weak_ptr<render::virtual_texture_registry> p_virtual_texture_registry
) noexcept -> void
{
    const auto serializer = virtual_texture_registry_json_serializer{
        std::move(p_cache_file_path),
        p_virtual_texture_registry
    };
    serializer.serialize_version_1();
}

auto virtual_texture_registry_json_serializer::deserialize(
    std::filesystem::path p_cache_file_path
) noexcept -> std::unique_ptr<render::virtual_texture_registry>
{
    auto virtual_texture_registry = render::virtual_texture_registry::create();
    auto serializer = virtual_texture_registry_json_serializer{
        std::move(p_cache_file_path),
        weak_ptr{ virtual_texture_registry.get() }
    };
    serializer.deserialize();
    return std::move(virtual_texture_registry);
}

auto virtual_texture_registry_json_serializer::serialize_version_1() const noexcept -> void
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
            std::string_view{
                render::virtual_texture_registry_serializer::k_registry_cache_document_type
            }
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
            for (const auto& [raw_handle, raw_metadata] : m_virtual_texture_registry->m_texture_metadata_map)
            {
                // skip missing texture as engine reloads automatically
                // TODO: evaluate if we should just store instead...
                if (raw_handle == m_virtual_texture_registry->m_missing_texture_data.m_raw_texture_handle)
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
            for (const auto& [virtual_handle, virtual_texture_data] : m_virtual_texture_registry->m_virtual_textures)
            {
                // skip missing texture as engine reloads automatically
                // TODO: evaluate if we should just store instead...
                if (virtual_handle == m_virtual_texture_registry->m_missing_texture_data.m_virtual_texture.m_handle)
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
            for (const auto& [virtual_handle, raw_handle] : m_virtual_texture_registry->m_virtual_to_raw_handle_map)
            {
                // skip missing texture as engine reloads automatically
                // TODO: evaluate if we should just store instead...
                if (virtual_handle == m_virtual_texture_registry->m_missing_texture_data.m_virtual_texture.m_handle)
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
        const auto path_str = m_cache_registry_file_path.string();
#ifdef KB_PLATFORM_WINDOWS
        constexpr const char* k_file_write_mode = "wb";
#else
        constexpr const char* k_file_write_mode = "w";
#endif

        // apparently c file api is more performant than c++ streams
        // according to rapidjson docs: https://rapidjson.org/md_doc_stream.html
        FILE* fp = fopen(path_str.c_str(), k_file_write_mode);
        KB_CORE_ASSERT(fp, "[virtual_texture_registry_serializer]: Failed to open cache document file '{}'", path_str);

        char write_buffer[65536];
        rapidjson::FileWriteStream output_stream{ fp, write_buffer, sizeof(write_buffer) };

        rapidjson::Writer writer{ output_stream };
        cache_document.Accept(writer);

        auto ret = fclose(fp);
        if (ret > 0)
        {
            KB_CORE_ERROR("[virtual_texture_registry_serializer]: Failed to close cache document file pointer");
        }
    }
}

// TODO: move to serializer class
auto virtual_texture_registry_json_serializer::deserialize() noexcept -> void
{
    const auto path_str = m_cache_registry_file_path.string();
    KB_CORE_INFO("[virtual_texture_registry_serializer]: Deserializing json cache document from '{}'", path_str);

    constexpr const char* k_file_read_mode = "r";

    std::ifstream cache_file_stream{ m_cache_registry_file_path, std::ios::in };
    if (!cache_file_stream)
    {
        KB_CORE_ASSERT(
            false,
            "[virtual_texture_registry_serializer]: Failed to open cache document '{}'!",
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

    if (!cache_document_type || cache_document_type != std::string_view{render::virtual_texture_registry_serializer::k_registry_cache_document_type })
    {
        KB_CORE_ERROR(
            "[virtual_texture_registry_serializer]: Document type could not be parsed or is not '{}'!",
            render::virtual_texture_registry_serializer::k_registry_cache_document_type
        );
        return;
    }

    const auto cache_document_version = util::get_value_from_document<u32>(
        cache_document,
        "version"
    );

    if (!cache_document_version)
    {
        KB_CORE_ERROR(
            "[virtual_texture_registry_serializer]: Document version could not be parsed!"
        );
    }

    switch (*cache_document_version)
    {
    case 1:
        deserialize_version_1();
        break;
    default:
        KB_CORE_ERROR("[virtual_texture_registry_serializer]: Unhandled cache document version '{}'", *cache_document_version);
        break;
    }

    // TODO: convert older schema versions

    if (*cache_document_version != render::virtual_texture_registry_serializer::k_registry_cache_json_latest_version)
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
auto virtual_texture_registry_json_serializer::deserialize_version_1() noexcept -> void
{
    // load raw texture data
    {
        if (m_document["raw_textures"].IsArray())
        {
            const auto raw_textures_data = m_document["raw_textures"].GetArray();
            for (auto&& texture_metadata : raw_textures_data)
            {
                // validation
                if (!texture_metadata["raw_handle"].IsString())
                {
                    KB_CORE_ERROR(
                        "[virtual_texture_registry_serializer]: Failed to read raw texture handle from raw texture metadata!"
                    );
                    continue;
                }

                if (!texture_metadata["texture_path"].IsString())
                {
                    KB_CORE_ERROR(
                        "[virtual_texture_registry_serializer]: Failed to read raw texture path from raw texture metadata!"
                    );
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
                    KB_CORE_ERROR(
                        "[virtual_texture_registry_serializer]: raw handle value '{}' is malformed!",
                        raw_handle_json
                    );
                    continue;
                }

                const auto raw_handle = raw_texture_handle{ raw_handle_val };

                KB_CORE_TRACE(
                    "[virtual_texture_registry_serializer]: Read raw texture data raw_handle={}, texture_path={}",
                    static_cast<raw_texture_handle::value_t>(raw_handle),
                    texture_path_cstr
                );

                m_virtual_texture_registry->m_texture_metadata_map.emplace(
                    raw_handle,
                    render::texture_metadata{
                        .m_path = std::move(texture_path),
                    }
                );
            }
        }
        else
        {
            KB_CORE_ERROR("[virtual_texture_registry_serializer]: Failed to read raw textures array data!");
        }
    }

    // load virtual texture data
    {
        const auto virtual_texture_json_data = m_document["raw_textures"].GetArray();
        for (auto&& virtual_texture_json : virtual_texture_json_data)
        {
            if (!virtual_texture_json["virtual_handle"].IsString())
            {
                KB_CORE_ERROR(
                    "[virtual_texture_registry_serializer]: Failed to read virtual handle from virtual texture metadata!"
                );
                continue;
            }

            if (!virtual_texture_json["uvs"].IsArray())
            {
                KB_CORE_ERROR(
                    "[virtual_texture_registry_serializer]: Failed to read uv array data from virtual texture metadata!"
                );
                continue;
            }

            auto uvs_json = virtual_texture_json["uvs"].GetArray();
            if (uvs_json.Size() != 4)
            {
                KB_CORE_ERROR(
                    "[virtual_texture_registry_serializer]: Expected 4 values in virtual texture uvs array, found {} instead?",
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
                    KB_CORE_ERROR("[virtual_texture_registry_serializer]: virtual handle value '{}' is malformed!", virtual_handle_json);
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
                        "[virtual_texture_registry_serializer]: Could not read uv x value!"
                    );
                    invalid_uv_data = true;
                    break;
                }

                if (!uv_json["y"].IsNumber())
                {
                    KB_CORE_ERROR(
                        "[virtual_texture_registry_serializer]: Could not read uv y value!"
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
                        KB_CORE_ERROR("[virtual_texture_registry_serializer]: uv x value '{}' is malformed!", x_json);
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
                        KB_CORE_ERROR("[virtual_texture_registry_serializer]: uv y value '{}' is malformed!", y_val);
                        invalid_uv_data = true;
                        continue;
                    }
                }

                uvs[uv_index++] = vec2_packed{ x_val, y_val };
            }

            if (invalid_uv_data)
                continue;

            // add virtual texture data to map
            m_virtual_texture_registry->m_virtual_textures.emplace(
                virtual_handle,
                render::virtual_texture_t{
                    .m_handle = virtual_handle,
                    .m_uvs = uvs
                }
            );
        }
    }

    // load virtual to raw handle map
    {
        const auto raw_handles_json_data = m_document["virtual_to_raw_handles"].GetArray();
        for (auto&& virtual_to_raw_json_data : raw_handles_json_data)
        {
            // validate virtual handle json
            if (!virtual_to_raw_json_data["virtual_handle"].IsString())
            {
                KB_CORE_ERROR(
                    "[virtual_texture_registry_serializer]: Could not read virtual handle in virtual to raw texture handles!"
                );
                return;
            }

            if (!virtual_to_raw_json_data["raw_handle"].IsString())
            {
                KB_CORE_ERROR(
                    "[virtual_texture_registry_serializer]: Could not read rwa handle in virtual to raw texture handles!"
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
                    KB_CORE_ERROR("[virtual_texture_registry_serializer]: virtual handle value '{}' is malformed!", virtual_handle_json);
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
                    KB_CORE_ERROR("[virtual_texture_registry_serializer]: raw handle value '{}' is malformed!", raw_handle_json);
                    continue;
                }
            }

            // store virtual to raw mapping
            m_virtual_texture_registry->m_virtual_to_raw_handle_map.emplace(
                virtual_texture_handle{ virtual_handle_hashed_val },
                raw_texture_handle{ raw_handle_hashed_val }
            );
        }
    }
}

} // end namespace kb::serialize
