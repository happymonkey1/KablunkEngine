#pragma once
#ifndef KABLUNK_ASSET_ASSET_COMMAND_H
#define KABLUNK_ASSET_ASSET_COMMAND_H

#include "Kablunk/Asset/AssetManager.h"

#include "Kablunk/Core/RefCounting.h"
#include "Kablunk/Core/Singleton.h"

#include "Kablunk/Core/Logger.h"

namespace Kablunk::asset
{
	template <typename T>
	IntrusiveRef<T> get_asset(const std::filesystem::path& filepath)
	{
		return Singleton<AssetManager>::get().get_asset<T>(filepath);
	}

	template <typename T>
	IntrusiveRef<T> get_asset(const char* filepath_cstr)
	{
		return Singleton<AssetManager>::get().get_asset<T>(std::filesystem::path{ filepath_cstr });
	}

	template <typename T>
	IntrusiveRef<T> get_asset(const asset_id_t& id)
	{
		return Singleton<AssetManager>::get().get_asset<T>(id);
	}

	inline bool asset_exists(const std::filesystem::path& filepath)
	{
		return Singleton<AssetManager>::get().asset_exists(filepath);
	}

	inline const AssetMetadata& try_get_asset_metadata(const std::filesystem::path& filepath)
	{
		return Singleton<AssetManager>::get().get_metadata(filepath);
	}

	inline const AssetMetadata& try_get_asset_metadata(const asset_id_t& id)
	{
		return Singleton<AssetManager>::get().get_metadata(id);
	}

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
			return "";

		return Singleton<AssetManager>::get().get_relative_path(asset_metadata.filepath);
	}

	// return the relative path for an asset by filepath
	inline std::filesystem::path get_relative_path(const std::filesystem::path& asset_path)
	{
		const asset::AssetMetadata& asset_metadata = asset::try_get_asset_metadata(asset_path);
		// #TODO assert that the path must be a valid asset
		if (!asset_metadata.is_valid())
			KB_CORE_WARN("[AssetManager]: path '{}' is not a valid asset!", asset_path);

		return Singleton<AssetManager>::get().get_relative_path(asset_path);
	}
}

#endif
