#pragma once

#ifndef KABLUNK_ASSET_ASSET_METADATA_H
#define KABLUNK_ASSET_ASSET_METADATA_H

#include "Kablunk/Core/Uuid64.h"
#include "Kablunk/Asset/AssetType.h"

#include <filesystem>

namespace Kablunk::asset
{

	struct AssetMetadata
	{
		// unique id for the asset
		uuid::uuid64 id = null_asset_id;
		// type of asset
		AssetType type = AssetType::NONE;
		// relative (to asset directory) path
		std::filesystem::path filepath = "";
		// #TODO
		bool is_memory_loaded = false;
		// #TODO
		bool is_data_loaded = false;

		// check whether the metadata is valid
		bool is_valid() const { return id != uuid::nil_uuid && type != AssetType::NONE; }
	};

}

#endif
