#pragma once
#ifndef KABLUNK_SERIALIZE_RENDERER_VIRTUAL_TEXTURE_JSON_SERIALIZER_H
#define KABLUNK_SERIALIZE_RENDERER_VIRTUAL_TEXTURE_JSON_SERIALIZER_H

#include <rapidjson/document.h>

namespace kb::render
{ // start namespace kb::render

// Forward declaration
class virtual_texture_registry;

} // end namespace kb::render

namespace kb::serialize
{ // start namespace kb::serialize

class virtual_texture_registry_json_serializer
{
public:
    inline static constexpr u32 k_json_schema_version = 1;

public:
    virtual_texture_registry_json_serializer() noexcept = default;
    ~virtual_texture_registry_json_serializer() noexcept = default;

    static auto serialize(
        std::filesystem::path p_cache_file_path,
        weak_ptr<render::virtual_texture_registry> p_virtual_texture_registry
    ) noexcept -> void;

    [[nodiscard]] static auto deserialize(
        std::filesystem::path p_cache_file_path
    ) noexcept -> std::unique_ptr<render::virtual_texture_registry>;

private:
    virtual_texture_registry_json_serializer(
        std::filesystem::path p_cache_file_path,
        weak_ptr<render::virtual_texture_registry> p_virtual_texture_registry
    )
        : m_virtual_texture_registry{ p_virtual_texture_registry },
        m_cache_registry_file_path{ std::move(p_cache_file_path) }
    {
    }

    // handler for serializing json data with document version 1 to disk
    auto serialize_version_1() const noexcept -> void;
    // handler for deserializing registry cache data from json
    auto deserialize() noexcept -> void;
    // handler for deserializing version 1 documents
    auto deserialize_version_1() noexcept -> void;

private:
    // Pointer to the virtual texture registry
    weak_ptr<render::virtual_texture_registry> m_virtual_texture_registry = nullptr;
    // File path for the cached virtual texture registry
    std::filesystem::path m_cache_registry_file_path{};
    // rapidjson Document
    rapidjson::Document m_document{};
};

} // end namespace kb::serialize

#endif
