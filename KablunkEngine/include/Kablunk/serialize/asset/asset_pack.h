#pragma once

#include "Kablunk/Asset/Asset.h"

#include "Kablunk/Core/owning_buffer.h"
#include "Kablunk/Core/Core.h"

#include "Kablunk/Project/Project.h"

#include "Kablunk/Scene/Scene.h"

#include "Kablunk/serialize/asset/asset_pack_file.h"
#include "Kablunk/serialize/asset/asset_pack_serializer.h"

#include <filesystem>

namespace kb::serialize::asset
{ // start namespace kb::serialize
class asset_pack : public RefCounted
{
public:
    asset_pack() = default;
    asset_pack(const std::filesystem::path& p_path);
    ~asset_pack() override = default;

    auto add_asset(ref<kb::asset::Asset> p_asset) -> void;

    auto serialize() -> void;

    auto load_scene(kb::asset::asset_id_t p_scene_id) -> ref<Scene>;
    auto load_asset(
        kb::asset::asset_id_t p_scene_id,
        kb::asset::asset_id_t p_asset_id
    ) -> ref<kb::asset::Asset>;

    // check whether a given asset id is valid
    inline auto is_asset_id_valid(kb::asset::asset_id_t p_asset_id) const -> bool
    {
        return m_asset_handle_index.contains(p_asset_id);
    }

    // check whether a given scene is valid, and the given asset is valid 
    // within the scene
    auto is_asset_id_valid(
        kb::asset::asset_id_t p_scene_id,
        kb::asset::asset_id_t p_asset_id
    ) const -> bool;

    auto read_app_binary() -> owning_buffer;
    inline auto get_build_version() -> uint64_t { return m_asset_pack_file.m_file_header.m_build_version; }

    static auto create_from_project(
        ref<Project> p_project,
        std::atomic<float>& p_progress
    ) -> ref<asset_pack>;
    static auto load(const std::filesystem::path& p_path) -> ref<asset_pack>;
    static auto load_into_project(ref<Project> p_project) -> ref<asset_pack>;
private:
    // path to the serialized asset pack file
    std::filesystem::path m_path{};
    // in-memory asset pack file
    asset_pack_file m_asset_pack_file{};
    // asset pack file serializer
    asset_pack_serializer m_asset_pack_serializer{};
    // set of asset ids included in the asset pack
    std::unordered_set<kb::asset::asset_id_t> m_asset_handle_index{};
};

} // end namespace kb::serialize
