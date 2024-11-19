#include "kablunkpch.h"

#include "Kablunk/serialize/renderer/virtual_texture_registry_yaml_serializer.h"

#include <yaml-cpp/yaml.h>

#include "Kablunk/renderer/virtual_texture_registry.h"

namespace kb::serialize
{ // start namespace kb::serialize

auto virtual_texture_registry_yaml_serializer::serialize(
    std::filesystem::path p_virtual_texture_registry_file_path,
    const weak_ptr<render::virtual_texture_registry> p_virtual_texture_registry
) noexcept -> void
{
    const auto serializer = virtual_texture_registry_yaml_serializer{
        std::move(p_virtual_texture_registry_file_path),
        p_virtual_texture_registry
    };
    serializer.serialize_version_1();
}

auto virtual_texture_registry_yaml_serializer::deserialize(
    std::filesystem::path p_virtual_texture_registry_file_path
) noexcept -> std::unique_ptr<render::virtual_texture_registry>
{
    auto virtual_texture_registry = render::virtual_texture_registry::create();
    auto serializer = virtual_texture_registry_yaml_serializer{
        std::move(p_virtual_texture_registry_file_path),
        weak_ptr{ virtual_texture_registry.get() }
    };
    serializer.deserialize();

    return serializer.m_valid ? std::move(virtual_texture_registry) : nullptr;
}

auto virtual_texture_registry_yaml_serializer::serialize_version_1() const noexcept -> void
{
    constexpr u32 k_version = 1ull;

    YAML::Emitter out{};

    out << YAML::BeginMap; // Begin root

    out << YAML::Key << "virtual_texture_registry" << YAML::BeginMap;

    out << YAML::Key << "version" << YAML::Value << k_version;

    out << YAML::Key << "raw_textures" << YAML::Value << YAML::BeginMap;
    for (const auto& [raw_texture_handle, texture_metadata] : m_virtual_texture_registry->m_texture_metadata_map)
    {
        out << YAML::Key << raw_texture_handle.as<u32>() << YAML::BeginMap;
        out << YAML::Key << "filepath" << YAML::Value << texture_metadata.m_path;
        out << YAML::EndMap; // End individual raw_texture map
    }
    out << YAML::EndMap; // End raw_textures

    out << YAML::Key << "virtual_textures" << YAML::Value << YAML::BeginMap;
    for (const auto& [virtual_texture_handle, virtual_texture_data] : m_virtual_texture_registry->m_virtual_textures)
    {
        out << YAML::Key << virtual_texture_handle.as<u32>() << YAML::Value << YAML::BeginMap;

        out << YAML::Key << "uvs" << YAML::Value << YAML::BeginMap;
        u32 index = 0;
        for (const auto& uv : virtual_texture_data.m_uvs)
        {
            out << YAML::Key << index++ << YAML::Value << YAML::BeginMap;
            out << YAML::Key << "x" << YAML::Value << YAML::Precision(6) << uv.m_storage.m_data[0];
            out << YAML::Key << "y" << YAML::Value << YAML::Precision(6) << uv.m_storage.m_data[1];
            out << YAML::EndMap; // End individual uv coordinate map
        }
        out << YAML::EndMap; // End UVs map

        out << YAML::Key << "dimensions" << YAML::Value << YAML::BeginMap;
        out << YAML::Key << "width" << YAML::Value << virtual_texture_data.get_width();
        out << YAML::Key << "height" << YAML::Value << virtual_texture_data.get_height();
        out << YAML::EndMap; // End dimensions map

        out << YAML::EndMap; // End virtual texture data
    }
    out << YAML::EndMap; // End virtual_textures

    out << YAML::Key << "virtual_to_raw_handles" << YAML::Value << YAML::BeginMap;
    for (const auto& [virtual_texture_handle, raw_texture_handle] : m_virtual_texture_registry->m_virtual_to_raw_handle_map)
    {
        out << YAML::Key << virtual_texture_handle.as<u32>() << YAML::Value << raw_texture_handle.as<u32>();
    }
    out << YAML::EndMap; // End virtual_to_raw_handles

    out << YAML::EndMap; // End virtual_texture_registry

    out << YAML::EndMap; // End root

    // Write to file
    std::ofstream file_out{ m_file_path };
    file_out << out.c_str();
}

auto virtual_texture_registry_yaml_serializer::deserialize() noexcept -> void
{
    KB_CORE_INFO("[virtual_texture_registry]: Deserializing cached registry from '{}'", m_file_path.string());
    const std::ifstream stream{ m_file_path };
    if (!stream)
    {
        KB_CORE_ASSERT(
            false,
            "[virtual_texture_registry_yaml_serializer]: Could not open cached yaml virtual texture registry '{}'",
            m_file_path.string()
        );
        return;
    }

    std::stringstream string_stream{};
    string_stream << stream.rdbuf();

    YAML::Node root;
    try
    {
        root = YAML::Load(string_stream.str());
        if (!root["virtual_texture_registry"])
        {
            KB_CORE_ASSERT(
                false,
                "[virtual_texture_registry_yaml_serializer]: Cannot deserialize virtual_texture_registry!"
            );
            return;
        }

        root = root["virtual_texture_registry"];
    }
    catch (YAML::ParserException& e)
    {
        KB_CORE_ERROR("Failed to deserialize virtual texture registry file '{0}'", m_file_path.string());
        KB_CORE_ERROR("    {0}", e.msg);
        return;
    }

    switch (const u32 document_version = root["version"].as<u32>())
    {
    case 1:
    {
        deserialize_version_1(root);
        return;
    }
    default:
    {
        KB_CORE_ASSERT(
            false,
            "[virtual_texture_registry_yaml_serializer]: Unhandled yaml documnet deserializer {}",
            document_version
        );
        return;
    }
    }
}

auto virtual_texture_registry_yaml_serializer::deserialize_version_1(YAML::Node& p_root) noexcept -> void
{
    // Load raw texture data from cached file
    {
        if (!p_root["raw_textures"])
        {
            KB_CORE_ASSERT(
                false,
                "[virtual_texture_registry_yaml_serializer]: Could not load raw textures from cached virtual texture registry"
            );

            m_valid = false;
            return;
        }

        const auto& raw_textures_node = p_root["raw_textures"];

        for (const auto& raw_texture_data_pair : raw_textures_node)
        {
            u32 raw_texture_handle_value;
            try
            {
                raw_texture_handle_value = raw_texture_data_pair.first.as<u32>();
            }
            catch (YAML::InvalidNode& err)
            {
                KB_CORE_ASSERT(
                    false,
                    "[virtual_texture_registry_yaml_serializer]: Failed to read raw texture handle from cached texture registry while reading `raw_textures`. Error={}",
                    err.what()
                );

                m_valid = false;
                return;
            }

            const auto& raw_texture_data_node = raw_texture_data_pair.second;
            if (!raw_texture_data_node["filepath"])
            {
                KB_CORE_ASSERT(
                    false,
                    "[virtual_texture_registry_yaml_serializer]: Failed to read filepath from raw texture handle data for raw_texture_handle='{}'",
                    raw_texture_handle_value
                );

                m_valid = false;
                return;
            }

            auto filepath = raw_texture_data_node["filepath"].as<std::string>();

            m_virtual_texture_registry->m_raw_textures.emplace(
                kb::raw_texture_handle{ raw_texture_handle_value },
                render::backend::texture_2d::create(filepath)
            );

            m_virtual_texture_registry->m_texture_metadata_map.emplace(
                kb::raw_texture_handle{ raw_texture_handle_value },
                render::texture_metadata_t{
                    .m_path = std::move(filepath)
                }
            );
        }
    }

    // Load virtual texture data from cached file
    {
        if (!p_root["virtual_textures"])
        {
            KB_CORE_ASSERT(
                false,
                "[virtual_texture_registry_yaml_serializer]: Could not load virtual textures from cached virtual texture registry"
            );

            m_valid = false;
            return;
        }

        const auto& virtual_textures_node = p_root["virtual_textures"];

        for (const auto& virtual_texture_data_pair : virtual_textures_node)
        {
            u32 virtual_texture_handle_value;
            try
            {
                virtual_texture_handle_value = virtual_texture_data_pair.first.as<u32>();
            }
            catch (YAML::InvalidNode& err)
            {
                KB_CORE_ASSERT(
                    false,
                    "[virtual_texture_registry_yaml_serializer]: Failed to read virtual texture handle from cached texture registry while reading `virtual_textures`. Error={}",
                    err.what()
                );

                m_valid = false;
                return;
            }

            const auto& virtual_texture_data_node = virtual_texture_data_pair.second;

            const auto& uv_array_node_data = virtual_texture_data_node["uvs"];

            std::array<vec2_packed, 4> uvs{};
            for (const auto& uvs_node_pair : uv_array_node_data)
            {
                u32 uv_index;
                try
                {
                    uv_index = uvs_node_pair.first.as<u32>();
                }
                catch (YAML::InvalidNode& err)
                {
                    KB_CORE_ASSERT(
                        false,
                        "[virtual_texture_registry_yaml_serializer]: Failed to read virtual uv data from cached texture registry while reading virtual_texture={}. Error={}",
                        virtual_texture_handle_value,
                        err.what()
                    );

                    m_valid = false;
                    return;
                }

                KB_CORE_ASSERT(
                    uv_index < 4,
                    "[virtual_texture_registry_yaml_serializer]: Failed to read virtual uv data, uv_index={} is out of uv array bounds = '4'",
                    uv_index
                );

                const auto& uvs_node_data = uvs_node_pair.second;

                if (!uvs_node_data["x"])
                {
                    KB_CORE_ASSERT(
                        false,
                        "[virtual_texture_registry_yaml_serializer]: Failed to read uv 'x' for uv_index={} and virtual_texture_handle={}",
                        uv_index,
                        virtual_texture_handle_value
                    );

                    m_valid = false;
                    return;
                }

                if (!uvs_node_data["y"])
                {
                    KB_CORE_ASSERT(
                        false,
                        "[virtual_texture_registry_yaml_serializer]: Failed to read uv 'y' for uv_index={} and virtual_texture_handle={}",
                        uv_index,
                        virtual_texture_handle_value
                    );

                    m_valid = false;
                    return;
                }

                uvs[uv_index] = { uvs_node_data["x"].as<f32>(), uvs_node_data["y"].as<f32>() };
            }

            if (!virtual_texture_data_node["dimensions"])
            {
                KB_CORE_ASSERT(
                    false,
                    "[virtual_texture_registry_yaml_serializer]: Failed to read dimensions node for virtual_texture_handle={}",
                    virtual_texture_handle_value
                );

                m_valid = false;
                return;
            }

            const auto& dimensions_node = virtual_texture_data_node["dimensions"];

            if (!dimensions_node["width"])
            {
                KB_CORE_ASSERT(
                    false,
                    "[virtual_texture_registry_yaml_serializer]: Failed to read width from dimensions node for virtual_texture_handle={}",
                    virtual_texture_handle_value
                );

                m_valid = false;
                return;
            }

            if (!dimensions_node["height"])
            {
                KB_CORE_ASSERT(
                    false,
                    "[virtual_texture_registry_yaml_serializer]: Failed to read width from dimensions node for virtual_texture_handle={}",
                    virtual_texture_handle_value
                );

                m_valid = false;
                return;
            }


            const kb::virtual_texture_handle virtual_texture_handle{ virtual_texture_handle_value };
            m_virtual_texture_registry->m_virtual_textures.emplace(
                virtual_texture_handle,
                render::virtual_texture_t{
                    .m_handle = virtual_texture_handle,
                    .m_uvs = uvs,
                    .m_dimensions = uvec2_packed{
                        dimensions_node["width"].as<u32>(),
                        dimensions_node["height"].as<u32>()
                    }
                }
            );
        }
    }

    // Load virtual to raw texture data from cached file
    {
        if (!p_root["virtual_to_raw_handles"])
        {
            KB_CORE_ASSERT(
                false,
                "[virtual_texture_registry_yaml_serializer]: Could not load virtual to raw handle data from cached virtual texture registry"
            );

            m_valid = false;
            return;
        }

        const auto virtual_to_raw_handle_node = p_root["virtual_to_raw_handles"];

        for (const auto& virtual_to_raw_handle_pair : virtual_to_raw_handle_node)
        {
            u32 virtual_handle;
            try
            {
                virtual_handle = virtual_to_raw_handle_pair.first.as<u32>();
            }
            catch (YAML::InvalidNode& err)
            {
                KB_CORE_ASSERT(
                    false,
                    "[virtual_texture_registry_yaml_serializer]: Failed to read virtual texture handle from cached texture registry while reading `virtual_to_raw_handles`. Error={}",
                    err.what()
                );

                m_valid = false;
                return;
            }

            const auto& raw_handle_from_virtual_node = virtual_to_raw_handle_pair.second;
            if (!raw_handle_from_virtual_node)
            {
                KB_CORE_ASSERT(
                    false,
                    "[virtual_texture_registry_yaml_serializer]: Failed to read raw handle for virtual_handle={} while reading `virtual_to_raw_handles`",
                    virtual_handle
                );

                m_valid = false;
                return;
            }

            const auto raw_handle = raw_handle_from_virtual_node.as<u32>();

            m_virtual_texture_registry->m_virtual_to_raw_handle_map.emplace(
                kb::virtual_texture_handle{ virtual_handle },
                kb::raw_texture_handle{ raw_handle }
            );
        }
    }
}

} // end namespace kb::serialize
