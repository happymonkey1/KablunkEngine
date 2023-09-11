#pragma once

#ifndef KABLUNK_ASSET_ASSET_METADATA_H
#define KABLUNK_ASSET_ASSET_METADATA_H

#include "Kablunk/Core/Uuid64.h"
#include "Kablunk/Asset/AssetType.h"

#include "Kablunk/Asset/AssetTypeDefs.h"

#include <filesystem>

namespace kb::asset
{

	struct AssetMetadata
	{
		// unique id for the asset
		uuid::uuid64 id = null_asset_id;
		// type of asset
		AssetType type = AssetType::NONE;
		// relative (to asset directory) path
		std::filesystem::path filepath = "";
		// flag for whether the asset only exists in memory
		bool is_memory_loaded = false;
		// flag for whether the asset has been loaded from disk
		bool is_data_loaded = false;
        // flag for whether the asset is an internal engine asset
        bool is_internal_asset = false;

		// check whether the metadata is valid
		bool is_valid() const { return id != uuid::nil_uuid && type != AssetType::NONE; }
	};

}

#endif
