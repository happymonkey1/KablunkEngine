#include "kablunkpch.h"

#include "Kablunk/serialize/asset/asset_pack.h"

#include "Kablunk/Core/Core.h"

namespace kb::serialize::asset
{ // start namespace kb::serialize::asset



asset_pack::asset_pack(const std::filesystem::path& p_path)
    : m_path{ p_path }, m_asset_pack_file{}, m_asset_pack_serializer{}, m_asset_handle_index{}
{

}

auto asset_pack::add_asset(arc<kb::asset::Asset> p_asset) -> void
{
    KB_CORE_ASSERT(false, "not implemented");
}

auto asset_pack::serialize() -> void
{
    KB_CORE_ASSERT(false, "not implemented");
}

auto asset_pack::load_scene(kb::asset::asset_id_t p_scene_id) -> arc<Scene>
{
    if (!m_asset_pack_file.m_index_table.m_scenes.contains(p_scene_id))
        return arc<Scene>{};

    const asset_pack_file::scene_header& scene_header_info = m_asset_pack_file.m_index_table.m_scenes.at(p_scene_id);

    KB_CORE_ASSERT(false, "not implemented!");
    // #TODO read scene data and deserialize into scene

    return arc<Scene>{};
}

auto asset_pack::load_asset(
    kb::asset::asset_id_t p_scene_id,
    kb::asset::asset_id_t p_asset_id
) -> arc<kb::asset::Asset>
{
    KB_CORE_ASSERT(false, "not implemented");
    return arc<kb::asset::Asset>{};
}

auto asset_pack::is_asset_id_valid(
    kb::asset::asset_id_t p_scene_id,
    kb::asset::asset_id_t p_asset_id
) const -> bool
{
    if (!m_asset_pack_file.m_index_table.m_scenes.contains(p_scene_id))
        return nullptr;

    const auto& scene_header_info = m_asset_pack_file.m_index_table.m_scenes.at(p_scene_id);
    return scene_header_info.m_asset_headers.contains(p_asset_id);
}

auto asset_pack::read_app_binary()->kb::owning_buffer
{
    KB_CORE_ASSERT(false, "not implemented");
    return owning_buffer{};
}

auto asset_pack::create_from_project(
    arc<Project> p_project,
    std::atomic<float>& p_progress
) -> arc<kb::serialize::asset::asset_pack>
{
    KB_CORE_ASSERT(false, "not implemented");
    return arc<asset_pack>{};
}

auto asset_pack::load(const std::filesystem::path& p_path) -> arc<kb::serialize::asset::asset_pack>
{
    auto new_asset_pack = arc<asset_pack>::Create();
    new_asset_pack->m_path = p_path;


    KB_CORE_ASSERT(false, "not implemented");
    return arc<asset_pack>{};
}

auto asset_pack::load_into_project(arc<Project> p_project) -> arc<kb::serialize::asset::asset_pack>
{
    KB_CORE_ASSERT(false, "not implemented");
    return arc<asset_pack>{};
}

} // end namespace kb::serialize::asset
