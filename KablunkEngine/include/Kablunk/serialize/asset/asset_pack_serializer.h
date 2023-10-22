#pragma once

#include "Kablunk/Core/Buffer.h"
#include "Kablunk/Core/Core.h"

#include "Kablunk/serialize/asset/asset_pack_file.h"

namespace kb::serialize::asset
{ // start namespace kb::serialize::asset

class asset_pack_serializer
{
public:
    void serialize(
        const std::filesystem::path& path, 
        asset_pack_file& p_asset_pack_file,
        Buffer p_app_binary,
        std::atomic<float>& p_progress
    );

    bool deserialize_index(
        const std::filesystem::path& p_path, 
        asset_pack_file& p_asset_pack_file
    );
private:
    uint64_t calculate_index_table_size(
        const asset_pack_file& p_asset_pack_file
    );
};

} // end namespace kb::serialize::asset
