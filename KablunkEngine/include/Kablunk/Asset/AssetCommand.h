#pragma once
#ifndef KABLUNK_ASSET_ASSET_COMMAND_H
#define KABLUNK_ASSET_ASSET_COMMAND_H

#include "Kablunk/Asset/AssetManager.h"

#include "Kablunk/Core/RefCounting.h"
#include "Kablunk/Core/Singleton.h"

#include "Kablunk/Core/Logger.h"

namespace Kablunk::asset
{
	// try find an asset by filepath
	template <typename T>
	IntrusiveRef<T> get_asset(const std::filesystem::path& filepath)
	{
		return Singleton<AssetManager>::get().get_asset<T>(filepath);
	}

	// try find an asset by filepath
	template <typename T>
	IntrusiveRef<T> get_asset(const char* filepath_cstr)
	{
		return Singleton<AssetManager>::get().get_asset<T>(std::filesystem::path{ filepath_cstr });
	}

	// try find an asset by id
	template <typename T>
	IntrusiveRef<T> get_asset(const asset_id_t& id)
	{
		return Singleton<AssetManager>::get().get_asset<T>(id);
	}

	// check whether an asset exists (by filepath)
	inline bool asset_exists(const std::filesystem::path& filepath)
	{
		return Singleton<AssetManager>::get().asset_exists(filepath);
	}

	// try get the metadata for an asset associated with the passed in filepath
	inline const AssetMetadata& try_get_asset_metadata(const std::filesystem::path& filepath)
	{
		return Singleton<AssetManager>::get().get_metadata(filepath);
	}

	// try get the metadata for an asset associated with the passed in id
	inline const AssetMetadata& try_get_asset_metadata(const asset_id_t& id)
	{
		return Singleton<AssetManager>::get().get_metadata(id);
	}

	// try get the asset type by filepath
	inline const AssetType get_asset_type_from_path(const std::filesystem::path& filepath)
	{
		return Singleton<AssetManager>::get().get_asset_type_from_filepath(filepath);
	}

	inline asset_id_t import_asset(const std::filesystem::path& filepath)
	{
		return Singleton<AssetManager>::get().import_asset(filepath);
	}

	template <typename T, typename... Args>
	ref<IAsset> create_asset(const std::filesystem::path& filepath, const std::filesystem::path& directory_path, Args&&... args)
	{
		return Singleton<AssetManager>::get().create_asset<T>(filepath, directory_path, args);
	}

	template <typename T, typename... Args>
	ref<IAsset> create_asset(const std::filesystem::path& filepath, Args&&... args)
	{
		return Singleton<AssetManager>::get().create_asset<T>(filepath.string(), std::filesystem::path{}, std::forward<Args>(args)...);
	}

	// return the absolute path for an asset. 
	inline std::filesystem::path get_absolute_path(const asset::asset_id_t& asset_id)
	{
		const asset::AssetMetadata& asset_metadata = asset::try_get_asset_metadata(asset_id);
		if (!asset_metadata.is_valid())
			return "";

		return Singleton<AssetManager>::get().get_absolute_path(asset_metadata.filepath);
	}

	// return the relative path for an asset by asset id.
	inline std::filesystem::path get_relative_path(const asset::asset_id_t& asset_id)
	{
		const asset::AssetMetadata& asset_metadata = asset::try_get_asset_metadata(asset_id);
		if (!asset_metadata.is_valid())
        {
            KB_CORE_ERROR("[asset command]: get_relative_path(asset_id={}) does not return valid metadata!", asset_id);
			return "";
        }

		return Singleton<AssetManager>::get().get_relative_path(asset_metadata.filepath);
	}

	// return the relative path for an asset by filepath
    // #TODO this function should not be exposed to the public api, all relative paths should be retrieved via an asset id
	inline std::filesystem::path get_relative_path(const std::filesystem::path& asset_path)
	{
		return Singleton<AssetManager>::get().get_relative_path(asset_path);
	}
}

#endif
