#pragma once

#include "Kablunk/Core/CoreTypes.h"
#include "Kablunk/Renderer/virtual_texture.h"
#include "Kablunk/Renderer/texture_handle.h"
#include "Kablunk/Renderer/Texture.h"

#include <filesystem>
#include <rapidjson/document.h>


namespace kb::asset
{ // start namespace kb::asset

// source texture type
enum class raw_texture_asset_type_t
{
    none = 0,
    texture_2d,
    texture_atlas,
};

enum class texture_registry_import_type_t
{
    none = 0,
    disk,
    memory,
};

struct virtual_texture_specification
{
    // path to the texture
    // value does not matter for textures not loaded from disk (`!texture_registry_import_type_t::disk`)
    std::filesystem::path m_path{};
    // configures registry import handler
    // how the registry should load source data
    texture_registry_import_type_t m_import_type = texture_registry_import_type_t::none;
    // whether the source ("raw") texture is a texture or atlas
    raw_texture_asset_type_t m_raw_texture_asset_type = raw_texture_asset_type_t::none;
    // flag for whether this texture can be (potentially) included in texture atlas packing
    bool m_allow_to_be_packed = false;
    // dimensions of the entire texture
    // optional if the texture import type is not atlas (`!raw_texture_asset_type_t::texture_atlas`)
    glm::vec2 m_texture_dimensions{};
    // dimensions of a sprite to extract from a texture atlas
    // value does not matter if the texture import type is not atlas (`!raw_texture_asset_type_t::texture_atlas`)
    glm::vec2 m_sprite_unpacker_dimensions{};
};

enum class serialization_type_t
{
    none = 0,
    json,
    bin,
};

struct virtual_texture_registry_specification
{
    std::filesystem::path m_cache_path = "assets/cache/";
    // flag for whether initialization should happen during construction
    bool m_initialize_on_construct = false;
    // serialization type (binary should be used for best performance in distribution builds)
    serialization_type_t m_serialization_type = serialization_type_t::json;
};

struct texture_metadata
{
    std::string m_path{};

    // return the size (in bytes) of the memory allocated
    [[nodiscard]] auto get_allocated_bytes() const noexcept -> size_t
    {
        // #TODO: not always accurate because of small string opt
        const auto path_alloc = m_path.capacity() * sizeof(char);
        return path_alloc;
    }
};

class virtual_texture_registry
{
public:
    struct debug_statistics
    {
        // total memory allocated for the asset manager
        u64 m_total_memory_allocated = 0;
        // memory allocated for internal use (data structures, etc.)
        u64 m_internal_memory_allocated = 0;
        // total memory allocated for raw textures (`Texture2D`)
        u64 m_raw_texture_memory_allocated = 0;
        // total memory allocated for virtual textures
        u64 m_virtual_texture_memory_allocated = 0;
        // total number of virtual textures created
        u64 m_total_raw_textures = 0;
        // total number of raw textures (`Texture2D`) created
        u64 m_total_virtual_textures = 0;
    };

    inline static constexpr const char* k_registry_cache_filename = "virtual_texture_registry.kbreg.json";
    inline static constexpr size_t k_registry_cache_json_latest_version = 1;
    inline static constexpr const char* k_registry_cache_document_type = "kb::virtual_texture_registry";
    inline static constexpr const char* k_missing_texture_file_path = "resources/textures/missing_texture.png";
    inline static constexpr const char* k_missing_texture_krn_cstr = "kb::texture::missing_texture";

public:
    virtual_texture_registry() = default;
    ~virtual_texture_registry() = default;

    [[nodiscard]] static auto create(
        const virtual_texture_registry_specification& p_specification
    ) -> std::unique_ptr<virtual_texture_registry>;

    [[nodiscard]] auto import(const virtual_texture_specification& p_specification) noexcept -> virtual_texture_handle;
    // process all imported textures
    // potentially creates texture atlases from standalone textures, which can invalidate previous handles
    auto process() noexcept -> void;

    // save the registry to a cache document
    auto save(
        const std::filesystem::path& p_cache_dir,
        serialization_type_t p_serialization_type = serialization_type_t::json
    ) const noexcept -> void;

    // load a cached version (cache document) of the registry
    auto load(
        const std::filesystem::path& p_cache_dir,
        serialization_type_t p_serialization_type = serialization_type_t::json
    ) noexcept -> void;

    [[nodiscard]] auto get_texture_2d_by_raw_handle(
        raw_texture_handle p_handle
    ) const noexcept -> const arc<Texture2D>&;
    [[nodiscard]] auto get_texture_2d_by_virtual_handle(
        virtual_texture_handle p_handle
    ) const noexcept -> const arc<Texture2D>&;

    [[nodiscard]] auto get_virtual_texture(
        virtual_texture_handle p_handle
    ) const noexcept -> const render::virtual_texture&;

    [[nodiscard]] auto has_virtual_texture(const virtual_texture_handle p_handle) const noexcept -> bool
    {
        return m_virtual_to_raw_handle_map.contains(p_handle) &&
            m_virtual_textures.contains(p_handle);
    }

    // scan virtual textures and check if there are any matches on `p_handle`
    [[nodiscard]] auto find_virtual_texture_by_raw_handle(const raw_texture_handle p_handle) const noexcept -> option<render::virtual_texture>
    {
        for (const auto& [virtual_handle, raw_handle] : m_virtual_to_raw_handle_map)
        {
            if (raw_handle == p_handle)
                return m_virtual_textures.at(virtual_handle);
        }

        return std::nullopt;
    }

    [[nodiscard]] auto get_debug_statistics() const noexcept -> debug_statistics;

    // returns an immutable reference to the underlying raw texture map
    [[nodiscard]] auto get_raw_texture_map() const noexcept -> const unordered_flat_map<raw_texture_handle, arc<Texture2D>>&
    {
        return m_raw_textures;
    }

    // returns an immutable reference to the underlying virtual texture map
    [[nodiscard]] auto get_virtual_texture_map() const noexcept ->
        const unordered_flat_map<virtual_texture_handle, render::virtual_texture>&
    {
        return m_virtual_textures;
    }

    // returns an immutable reference to the underlying virtual to raw handle map
    [[nodiscard]] auto get_virtual_to_raw_handle_map() const noexcept ->
        const unordered_flat_map<virtual_texture_handle, raw_texture_handle>&
    {
        return m_virtual_to_raw_handle_map;
    }
private:
    // handler that processes already imported standalone textures and creates texture atlas(es)
    auto create_texture_atlases() noexcept -> void;
    // handler for importing textures from disk
    auto import_texture_from_disk(const virtual_texture_specification& p_specification) noexcept -> raw_texture_handle;
    // load missing texture data
    auto import_missing_texture() noexcept -> void;

    auto create_or_get_virtual_texture(
        const virtual_texture_specification& p_specification,
        raw_texture_handle p_raw_texture_handle
    ) noexcept -> virtual_texture_handle;

    // handler for serializing json data to disk
    auto serialize_registry_json_data_to_disk(
        const std::filesystem::path& p_cache_registry_file_path,
        u32 p_version
    ) const noexcept -> void;
    // handler for serializing json data with document version 1 to disk
    auto serialize_registry_json_data_version_1_to_disk(
        const std::filesystem::path& p_cache_registry_file_path
    ) const noexcept -> void;
    // handler for deserializing registry cache data from json
    auto deserialize_registry_data_from_json(
        const std::filesystem::path& p_cache_json_file_path
    ) noexcept -> void;
    // handler for deserializing version 1 documents
    auto deserialize_registry_data_from_json_version_1(
        rapidjson::Document& p_document
    ) noexcept -> void;

private:
    // map of raw textures
    unordered_flat_map<raw_texture_handle, arc<Texture2D>> m_raw_textures{};
    // map of texture metadata
    unordered_flat_map<raw_texture_handle, texture_metadata> m_texture_metadata_map{};
    // map of virtual texture handle to raw texture handle
    unordered_flat_map<virtual_texture_handle, raw_texture_handle> m_virtual_to_raw_handle_map{};
    // map of virtual texture
    unordered_flat_map<virtual_texture_handle, render::virtual_texture> m_virtual_textures{};

    // missing texture
    struct missing_texture_data
    {
        // raw missing texture data
        arc<Texture2D> m_raw_texture{};
        // virtual texture data
        render::virtual_texture m_virtual_texture{};
        // raw texture handle
        raw_texture_handle m_raw_texture_handle{};
    } m_missing_texture_data{};

    // debug statistics
    debug_statistics m_debug_statistics{};
};

} // end namespace kb::asset
