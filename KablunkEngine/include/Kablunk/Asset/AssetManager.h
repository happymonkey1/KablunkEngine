#pragma once

#ifndef KABLUNK_ASSET_ASSET_MANAGER_H
#define KABLUNK_ASSET_ASSET_MANAGER_H

#include "Kablunk/Asset/AssetRegistry.h"
#include "Kablunk/Asset/Asset.h"
#include "Kablunk/Asset/AssetType.h"

#include "Kablunk/Project/Project.h"

#include <filesystem>

namespace Kablunk::asset
{

	class AssetManager
	{
	public:
		// initialization logic for the asset manager
		void init();
		// shutdown logic for the asset manager
		void shutdown();

		const AssetMetadata& get_metadata(const asset_id_t& id) const { return m_asset_registry.at(id); }
		const AssetMetadata& get_metadata(const std::filesystem::path& filepath) const;
		const AssetMetadata& get_metadata(const IntrusiveRef<IAsset>& asset) const { return get_metadata(asset->get_id()); }

		const std::filesystem::path& get_absolute_path(const AssetMetadata& metadata) const { return Project::GetAssetDirectoryPath() / metadata.filepath; }
		const std::filesystem::path& get_relative_path(const std::filesystem::path& path) const;

		AssetType get_asset_type_by_extension(const std::string& extension) const { return string_to_asset_type(extension); }
		AssetType get_asset_type_from_filepath(const std::filesystem::path& path) const;
		// import an asset into the registry
		const asset_id_t& import_asset(const std::filesystem::path& filepath);
		// reload a specific asset's data
		bool reload_asset_data(const asset_id_t& id);
		// get an asset id based on the filepath. O(N) time complexity
		const asset_id_t& find_asset_id_based_on_filepath(const std::filesystem::path& filepath) const;
		// create a new asset
		template <typename T, typename... Args>
		IntrusiveRef<T> create_asset(const std::string& filename, const std::filesystem::path& directory_path, Args&&... args)
		{
			static_assert(std::is_base_of<IAsset, T>::value, "create_asset() only works for types derived from IAsset!");

			AssetMetadata metadata{
				uuid::generate(),
				T::get_static_type(),
				directory_path.empty() || directory_path == "." ? filename : get_relative_path(directory_path / filename),
				false, // is_memory_loaded
				true,  // is_data_loaded
			};

			// #TODO check if asset already exists in registry

			m_asset_registry[metadata.id] = metadata;

			// #TODO should we write registry to file every time?
			write_registry_to_file();

			IntrusiveRef<T> asset = IntrusiveRef<T>::Create(std::forward<Args>(args)...);
			asset->set_id(metadata.id);
			m_loaded_assets[metadata.id] = asset;

			return asset;
		}
		// get an asset based off an asset id
		template <typename T>
		IntrusiveRef<T> get_asset(const asset_id_t& id)
		{
			static_assert(std::is_base_of<IAsset, T>::value, "get_asset() only works for types derived from IAsset!");

			if (is_memory_asset(id))
				return m_memory_assets.at(asset_id_t).As<T>();

			AssetMetadata& metadata = get_metadata(id);
			if (!metadata.is_valid())
				return nullptr;

			IntrusiveRef<T> asset = nullptr;
			if (!metadata.is_data_loaded)
			{
				// #TODO try load data
				KB_CORE_ASSERT(false, "not implemented!");
			}
			else
				asset = m_loaded_assets[id];

			return asset.As<T>();
		}
		// get an asset based on a filepath
		template <typename T>
		IntrusiveRef<T> get_asset(const std::filesystem::path& filepath) { return get_asset(find_asset_id_based_on_filepath(filepath)); }
		// check whether the asset referenced by the id is a memory only asset
		bool is_memory_asset(const asset_id_t& id) const { return m_memory_assets.find(id) != m_memory_assets.end(); }
		// get the map of loaded assets
		const std::unordered_map<asset_id_t, IntrusiveRef<IAsset>>& get_loaded_assets() const { return m_loaded_assets; }
		// get the map of memory only assets
		const std::unordered_map<asset_id_t, IntrusiveRef<IAsset>>& get_memory_assets() const { return m_memory_assets; }
		// get the underlying asset registry
		const AssetRegistry& get_asset_registry() const { return m_asset_registry; }
	private:
		void load_asset_registry();
		void write_registry_to_file() const;
		void reload_assets();
		void process_directory(const std::filesystem::path& directory_path);

		AssetMetadata& get_metadata(const asset_id_t& id);

		void on_asset_renamed(const asset_id_t& id, const std::filesystem::path& new_filepath);
		void on_asset_deleted(const asset_id_t& id);
	private:
		// asset registry that maps ids to metadata
		AssetRegistry m_asset_registry;
		// map of assets that are fully loaded
		std::unordered_map<asset_id_t, IntrusiveRef<IAsset>> m_loaded_assets;
		// map of assets loaded in memory
		std::unordered_map<asset_id_t, IntrusiveRef<IAsset>> m_memory_assets;
		// #TODO filesystem changed callback

		// null metadata for functions that return references
		inline static AssetMetadata s_null_metadata{};
	};

}

#endif
