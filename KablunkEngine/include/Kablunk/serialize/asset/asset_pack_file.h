#pragma once

#include "Kablunk/Core/Core.h"
#include "Kablunk/Asset/Asset.h"

namespace kb::serialize::asset
{ // start namespace kb::serialize::asset

struct asset_pack_file
{

    struct asset_header
    {
        u64 m_packed_offset = 0ull;
        u64 m_packed_size = 0ull;
        u16 m_type = 0ull;
        u16 m_flags = 0ull;
    };

    struct scene_header
    {
        u64 m_packed_offset = 0ull;
        u64 m_packed_size = 0ull;
        u16 m_flags = 0ull;
        std::map<kb::asset::asset_id_t, asset_header> m_asset_headers;
    };

    struct index_table
    {
        u64 m_packed_app_binary_offset = 0ull;
        u64 m_packed_app_binary_size = 0ull;
        std::map<u64, scene_header> m_scenes;
    };

    struct file_header
    {
        static constexpr const uint32_t k_version_number = 1000;
        const char m_magic_header[4] = { 'k', 'b', 'a', 'p' };
        uint32_t m_version = k_version_number;
        uint64_t m_build_version = 0;
    };

    file_header m_file_header;
    index_table m_index_table;
};

} // end namespace kb::serialize::asset
