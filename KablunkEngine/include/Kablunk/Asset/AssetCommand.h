#pragma once
#ifndef KABLUNK_ASSET_ASSET_COMMAND_H
#define KABLUNK_ASSET_ASSET_COMMAND_H

#include "Kablunk/Asset/AssetManager.h"

#include "Kablunk/Core/RefCounting.h"
#include "Kablunk/Core/Singleton.h"

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
}

#endif
