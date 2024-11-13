#pragma once
#ifndef KABLUNK_RENDERER_VIRTUAL_TEXTURE_REGISTRY_SERIALIZER_H
#define KABLUNK_RENDERER_VIRTUAL_TEXTURE_REGISTRY_SERIALIZER_H

#include "Kablunk/Core/CoreTypes.h"
#include "Kablunk/Core/RefCounting.h"

#include <filesystem>

namespace kb::render
{ // start namespace kb::render

// Forward declaration
class virtual_texture_registry;

struct virtual_texture_registry_serializer
{
    // Filename for the virtual texture registry cache, without the file extension
    inline static constexpr const char* k_registry_cache_filename = "virtual_texture_registry.kbreg";
    // Json schema version
    inline static constexpr size_t k_registry_cache_json_latest_version = 1ull;
    inline static constexpr const char* k_registry_cache_document_type = "kb::virtual_texture_registry";

    enum class serialization_type_t
    {
        yaml,
        json,
        bin,
    };

    // Serialize the virtual texture registry based on serialization type
    static auto serialize(
        serialization_type_t p_serialization_type,
        const std::filesystem::path& p_cache_dir,
        weak_ptr<virtual_texture_registry> p_virtual_texture_registry
    ) noexcept -> void;

    // Deserialize the virtual texture registry based on serialization type
    static auto deserialize(
        serialization_type_t p_serialization_type,
        const std::filesystem::path& p_cache_file_dir
    ) noexcept -> std::unique_ptr<virtual_texture_registry>;
};

} // end namespace kb::render

#endif
