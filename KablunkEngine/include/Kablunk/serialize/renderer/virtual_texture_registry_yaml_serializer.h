#pragma once
#ifndef KABLUNK_SERIALIZE_RENDERER_VIRTUAL_TEXTURE_REGISTRY_YAML_SERIALIZER_H
#define KABLUNK_SERIALIZE_RENDERER_VIRTUAL_TEXTURE_REGISTRY_YAML_SERIALIZER_H

#include <yaml-cpp/yaml.h>

#include <filesystem>
#include <memory>

namespace kb::render
{ // start namespace kb::render

// Forward declaration
class virtual_texture_registry;

} // end namespace kb::render

namespace kb::serialize
{ // start namespace kb::serialize

class virtual_texture_registry_yaml_serializer
{
public:
    static auto serialize(
        std::filesystem::path p_cache_file_path,
        weak_ptr<render::virtual_texture_registry> p_virtual_texture_registry
    ) noexcept -> void;

    [[nodiscard]] static auto deserialize(
        std::filesystem::path p_cache_file_path
    ) noexcept -> std::unique_ptr<render::virtual_texture_registry>;

private:
    virtual_texture_registry_yaml_serializer(
        std::filesystem::path p_virtual_texture_registry_file_path,
        const weak_ptr<render::virtual_texture_registry> p_virtual_texture_registry
    ) : m_virtual_texture_registry{ p_virtual_texture_registry },
        m_file_path{ std::move(p_virtual_texture_registry_file_path) }
    { }

    auto serialize_version_1() const noexcept -> void;
    auto deserialize() noexcept -> void;
    auto deserialize_version_1(YAML::Node& p_root) noexcept -> void;

private:
    weak_ptr<render::virtual_texture_registry> m_virtual_texture_registry;
    std::filesystem::path m_file_path;
    // Used during deserialization to flag an invalid cached file
    bool m_valid = true;
};

} // end namespace kb::serialize

#endif
