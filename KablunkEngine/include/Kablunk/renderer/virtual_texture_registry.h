#pragma once

#include "Kablunk/Core/CoreTypes.h"
#include "Kablunk/renderer/virtual_texture.h"
#include "Kablunk/renderer/texture_handle.h"
#include "Kablunk/renderer/backend/texture.h"
#include "Kablunk/renderer/virtual_texture_registry_serializer.h"

#include <filesystem>

namespace kb::serialize
{ // start namespace kb::serialize

// Forward declarations
class virtual_texture_registry_json_serializer;
class virtual_texture_registry_yaml_serializer;

} // end namespace kb::serialize

namespace kb::render
{ // start namespace kb::render

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

struct virtual_texture_specification_t
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
    glm::uvec2 m_texture_dimensions{};
    // dimensions of a sprite to extract from a texture atlas
    // value does not matter if the texture import type is not atlas (`!raw_texture_asset_type_t::texture_atlas`)
    glm::uvec2 m_sprite_unpacker_dimensions{};
    // Specification for loading a texture from memory
    // Not required if loading a texture from disk
    struct in_memory_specification_t
    {
        // Resource name used to generate the raw and virtual handles
        std::string m_name;
        // Pointer to image loaded into memory for an in-memory texture
        // Used when loading textures from memory
        const void* m_memory_data_ptr = nullptr;
        // Texture specification which contains details like image format
        backend::texture_specification_t m_texture_specification{};
    } m_in_memory_specification{};
};

struct texture_metadata_t
{
    std::string m_path{};
    bool m_is_atlas = false;
    bool m_is_memory_only = false;

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
    struct debug_statistics_t
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

    inline static constexpr const char* k_missing_texture_file_path = "resources/textures/missing_texture.png";
    inline static constexpr const char* k_missing_texture_krn_cstr = "kb::texture::missing_texture";
    inline static constexpr raw_texture_handle k_missing_texture_raw_handle = raw_texture_handle::into(std::string_view{ k_missing_texture_file_path });
    inline static constexpr virtual_texture_handle k_missing_texture_virtual_handle = virtual_texture_handle::into(
        std::string_view{ k_missing_texture_krn_cstr }
    );

public:
    virtual_texture_registry() noexcept = default;
    ~virtual_texture_registry() noexcept = default;

    [[nodiscard]] static auto create() noexcept -> std::unique_ptr<virtual_texture_registry>;

    [[nodiscard]] auto load_texture(
        const virtual_texture_specification_t& p_specification
    ) noexcept -> virtual_texture_handle;
    [[nodiscard]] auto load_texture_from_memory(
        std::string_view p_texture_name,
        const backend::texture_specification_t& p_texture_specification,
        const void* p_data,
        bool p_is_atlas
    ) noexcept -> virtual_texture_handle;
    [[nodiscard]] auto load_individual_texture(
        std::filesystem::path p_texture_path
    ) noexcept -> virtual_texture_handle;
    [[nodiscard]] auto load_texture_atlas(
        std::filesystem::path p_atlas_path
    ) noexcept -> virtual_texture_handle;

    auto release_texture(virtual_texture_handle p_handle) noexcept -> void;

    // Create a virtual texture handle from a filepath
    [[nodiscard]] static auto create_virtual_texture_handle(
        const std::filesystem::path& p_file_path
    ) noexcept -> virtual_texture_handle;

    // Create a virtual texture handle from a resource name
    [[nodiscard]] static auto create_virtual_texture_handle(
        std::string_view p_name
    ) noexcept -> virtual_texture_handle;

    // process all imported textures
    // potentially creates texture atlases from standalone textures, which can invalidate previous handles
    auto process() noexcept -> void;

    // save the registry to a cache document
    auto save(
        virtual_texture_registry_serializer::serialization_type_t p_serialization_type,
        const std::filesystem::path& p_cache_dir
    ) const noexcept -> void;

    // load a cached version (cache document) of the registry
    [[nodiscard]] static auto load(
        virtual_texture_registry_serializer::serialization_type_t p_serialization_type,
        const std::filesystem::path& p_cache_dir
    ) noexcept -> std::unique_ptr<virtual_texture_registry>;

    [[nodiscard]] auto get_texture_2d_by_raw_handle(
        raw_texture_handle p_handle
    ) const noexcept -> const arc<backend::texture_2d>&;
    [[nodiscard]] auto get_texture_2d_by_virtual_handle(
        virtual_texture_handle p_handle
    ) const noexcept -> const arc<backend::texture_2d>&;

    [[nodiscard]] auto get_virtual_texture(
        virtual_texture_handle p_handle
    ) const noexcept -> const virtual_texture_t&;

    [[nodiscard]] auto has_virtual_texture(const virtual_texture_handle p_handle) const noexcept -> bool
    {
        return m_virtual_to_raw_handle_map.contains(p_handle) &&
            m_virtual_textures.contains(p_handle);
    }

    // scan virtual textures and check if there are any matches on `p_handle`
    [[nodiscard]] auto find_virtual_texture_by_raw_handle(const raw_texture_handle p_handle) const noexcept -> option<virtual_texture_t>
    {
        for (const auto& [virtual_handle, raw_handle] : m_virtual_to_raw_handle_map)
        {
            if (raw_handle == p_handle)
                return m_virtual_textures.at(virtual_handle);
        }

        return std::nullopt;
    }

    [[nodiscard]] auto get_debug_statistics() const noexcept -> debug_statistics_t;

    // returns an immutable reference to the underlying raw texture map
    [[nodiscard]] auto get_raw_texture_map() const noexcept -> const unordered_flat_map<raw_texture_handle, arc<backend::texture_2d>>&
    {
        return m_raw_textures;
    }

    // returns an immutable reference to the underlying virtual texture map
    [[nodiscard]] auto get_virtual_texture_map() const noexcept ->
        const unordered_flat_map<virtual_texture_handle, virtual_texture_t>&
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
    auto import_texture_from_disk(const virtual_texture_specification_t& p_specification) noexcept -> raw_texture_handle;
    // handler for importing in-memory textures
    auto import_texture_from_memory(const virtual_texture_specification_t& p_specification) noexcept -> raw_texture_handle;
    // load missing texture data
    auto import_missing_texture() noexcept -> void;

    auto create_or_get_virtual_texture(
        const virtual_texture_specification_t& p_specification,
        raw_texture_handle p_raw_texture_handle
    ) noexcept -> virtual_texture_handle;

private:
    // map of raw textures
    unordered_flat_map<raw_texture_handle, arc<backend::texture_2d>> m_raw_textures{};
    // map of texture metadata
    unordered_flat_map<raw_texture_handle, texture_metadata_t> m_texture_metadata_map{};
    // map of virtual texture handle to raw texture handle
    unordered_flat_map<virtual_texture_handle, raw_texture_handle> m_virtual_to_raw_handle_map{};
    // map of virtual texture
    unordered_flat_map<virtual_texture_handle, virtual_texture_t> m_virtual_textures{};

    // missing texture
    struct missing_texture_data
    {
        // raw missing texture data
        arc<backend::texture_2d> m_raw_texture{};
        // virtual texture data
        virtual_texture_t m_virtual_texture{};
        // raw texture handle
        raw_texture_handle m_raw_texture_handle{};
    } m_missing_texture_data{};

    // debug statistics
    debug_statistics_t m_debug_statistics{};

    // Friend classes
    friend class ::kb::serialize::virtual_texture_registry_json_serializer;
    friend class ::kb::serialize::virtual_texture_registry_yaml_serializer;
};

} // end namespace kb::render
