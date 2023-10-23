#pragma once

#ifndef KABLUNK_ASSET_ASSET_MANAGER_H
#define KABLUNK_ASSET_ASSET_MANAGER_H

#include "Kablunk/Asset/AssetRegistry.h"
#include "Kablunk/Asset/Asset.h"
#include "Kablunk/Asset/AssetType.h"
#include "Kablunk/Asset/AssetSerializer.h"

#include "Kablunk/Project/Project.h"

#include <filesystem>

namespace kb::asset
{

	class AssetManager : public RefCounted
	{
	public:
		// initialization logic for the asset manager
		void init(ref<Project> p_active_project, bool p_load_internal_engine_assets = true);
		// shutdown logic for the asset manager
		void shutdown();
		// get metadata from asset registry using asset id
		const AssetMetadata& get_metadata(const asset_id_t& id) const { return m_asset_registry.contains(id) ? m_asset_registry.at(id) : s_null_metadata; }
		// get metadata from asset registry using filepath
		const AssetMetadata& get_metadata(const std::filesystem::path& filepath) const;
		// get metadata from asset registry using asset reference
		const AssetMetadata& get_metadata(const ref<IAsset>& asset) const { return get_metadata(asset->get_id()); }
		// get the absolute path for an asset using its metadata
		std::filesystem::path get_absolute_path(const AssetMetadata& metadata) const 
        { 
            if (metadata.filepath.is_absolute())
                return metadata.filepath;

            // #TODO internal engine path should not be hardcoded here...
            return metadata.is_internal_asset ? m_active_project->get_project_directory() / "resources" / metadata.filepath 
                : m_active_project->get_asset_directory_path() / metadata.filepath;
        }
		// get the absolute path for a given path
		std::filesystem::path get_absolute_path(const std::filesystem::path& path) const 
        { 
            return path.is_absolute() ? path : m_active_project->get_asset_directory_path() / path; 
        }
        // get the relative path for a given path stored in some asset metadata
        std::filesystem::path get_relative_path(const AssetMetadata& p_metadata) const;
		// get the relative path for a given path
		std::filesystem::path get_relative_path(const std::filesystem::path& path) const;
        // get the absolute path to the asset directory
        inline auto get_absolute_asset_directory_path() const -> std::filesystem::path
        {
            return m_active_project->get_asset_directory_path();
        }
		// get an asset type by a given extension
		AssetType get_asset_type_by_extension(const std::string& extension) const { return extension_to_asset_type(extension); }
		// get an asset type from a given path
		AssetType get_asset_type_from_filepath(const std::filesystem::path& path) const;
		// import an asset into the registry
		asset_id_t import_asset(const std::filesystem::path& filepath);
		// reload a specific asset's data
		bool reload_asset_data(const asset_id_t& id);
		// get an asset id based on the filepath. O(N) time complexity
		const asset_id_t& find_asset_id_based_on_filepath(const std::filesystem::path& filepath) const;
		// create a new asset
		template <typename T, typename... Args>
		ref<T> create_asset(const std::string& filename, const std::filesystem::path& directory_path, Args&&... args)
		{
			static_assert(std::is_base_of<IAsset, T>::value, "create_asset() only works for types derived from IAsset!");

			AssetMetadata metadata{
				uuid::generate(),
				T::get_static_type(),
				directory_path.empty() || directory_path == "." ? filename : get_relative_path(directory_path / filename),
				false, // is_memory_loaded
				true,  // is_data_loaded
			};

			// check if we need to modify filepath because it already exists in our project
			if (file_exists(metadata))
			{
				bool filename_available = false;
				size_t index = 1;

				while (!filename_available)
				{
					std::filesystem::path new_filepath = directory_path / metadata.filepath.stem();
					new_filepath += (index < 10) ? "(0" + std::to_string(index) + ")" : "(" + std::to_string(index) + ")";
					new_filepath += metadata.filepath.extension();

					if (!FileSystem::file_exists(Singleton<ProjectManager>::get().get_active()->get_asset_directory_path() / get_relative_path(new_filepath)))
					{
						filename_available = true;
						metadata.filepath = new_filepath;
						break;
					}

					// sanity check
					KB_CORE_ASSERT(index < 10000, "logic error!");
					index++;
				}
			}

			m_asset_registry[metadata.id] = metadata;

			// #TODO should we write registry to file every time?
			write_registry_to_file();

			ref<T> asset = ref<T>::Create(std::forward<Args>(args)...);
			asset->set_id(metadata.id);
			m_loaded_assets[metadata.id] = asset;

			serialize_asset(metadata, asset.As<IAsset>());

			return asset;
		}
		// get an asset based off an asset id
		template <typename T>
		ref<T> get_asset(const asset_id_t& id)
		{
			static_assert(std::is_base_of<IAsset, T>::value, "get_asset() only works for types derived from IAsset!");

			if (is_memory_asset(id))
				return m_memory_assets.at(id).As<T>();

			AssetMetadata& metadata = get_metadata_internal(id);
			if (!metadata.is_valid())
			{
				KB_CORE_INFO("[AssetManager] Tried to get asset '{}' but metadata is invalid!", id);
				return nullptr;
			}

			// imported assets are not loaded by default, so try to load if we find one that hasn't been loaded
			// #TODO async asset loading
			ref<IAsset> asset = nullptr;
			if (!metadata.is_data_loaded)
			{
				metadata.is_data_loaded = try_load_asset(metadata, asset);
				if (!metadata.is_data_loaded)
					return nullptr;

                KB_CORE_ASSERT(asset, "[asset_manager]: trying to emplace null asset into loaded asset registry?");

				m_loaded_assets[id] = asset;
			}
			else
				asset = m_loaded_assets[id];

			KB_CORE_ASSERT(asset, "asset null?!");

			//KB_CORE_INFO("[AssetManager] Successfully retrieved asset '{}'", metadata.filepath);
			return asset.As<T>();
		}
		// get an asset based on a filepath
		template <typename T>
		ref<T> get_asset(const std::filesystem::path& filepath) { return get_asset<T>(find_asset_id_based_on_filepath(filepath)); }
		// check whether the asset exists in the asset registry
		bool asset_exists(const std::filesystem::path& filepath) const;
		// check whether the asset referenced by the id is a memory only asset
		bool is_memory_asset(const asset_id_t& id) const { return m_memory_assets.find(id) != m_memory_assets.end(); }
		// get the map of loaded assets
		const kb::unordered_flat_map<asset_id_t, ref<IAsset>>& get_loaded_assets() const { return m_loaded_assets; }
		// get the map of memory only assets
		const kb::unordered_flat_map<asset_id_t, ref<IAsset>>& get_memory_assets() const { return m_memory_assets; }
		// get the underlying asset registry
		const AssetRegistry& get_asset_registry() const { return m_asset_registry; }
		// check if the file exists on the filesystem
		bool file_exists(const AssetMetadata& asset_metadata) const;
	private:
		// load asset registry file from disk
		void load_asset_registry();
		// save asset registry file to disk
		void write_registry_to_file() const;
		// process directories recursively (starting from asset dir root), import assets, then save registry file
		void reload_assets();
		void process_directory(const std::filesystem::path& directory_path);

		// write asset data to disk
		void serialize_asset(const AssetMetadata& metadata, ref<IAsset>& asset) const;
		// try load asset from disk
		bool try_load_asset(const AssetMetadata& metadata, ref<IAsset>& asset) const;

		AssetMetadata& get_metadata_internal(const asset_id_t& id);

		// callback for when an asset has been renamed on disk
		void on_asset_renamed(const asset_id_t& id, const std::filesystem::path& new_filepath);
		// callback for when an asset has been deleted from disk
		void on_asset_deleted(const asset_id_t& id);

        // import an engine asset's metadata into the asset registry
        asset_id_t import_engine_asset_metadata(const std::filesystem::path& filepath);
	public:
		// extension for registry file
		// #TODO constexpr in c++20?
		inline static const std::filesystem::path s_asset_registry_path = "asset_registry.kbreg";
	private:
        ref<Project> m_active_project = nullptr;
		// asset registry that maps ids to metadata
		AssetRegistry m_asset_registry;
		// map of assets that are fully loaded
		kb::unordered_flat_map<asset_id_t, ref<IAsset>> m_loaded_assets;
		// map of assets loaded in memory
		kb::unordered_flat_map<asset_id_t, ref<IAsset>> m_memory_assets;
		// #TODO filesystem changed callback

		// null metadata for functions that return references
		inline static AssetMetadata s_null_metadata{};
		// map for serializers of specific asset types
		kb::unordered_flat_map<AssetType, ref<AssetSerializer>> m_asset_serializers;
	};

	// check whether a given filepath refers to the asset registry
	inline bool is_asset_registry_file(const std::filesystem::path& filepath)
	{
		return filepath.filename() == AssetManager::s_asset_registry_path;
	}

}

#endif
