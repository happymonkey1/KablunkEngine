#include "kablunkpch.h"

#include "Kablunk/Asset/AssetManager.h"

#include "Kablunk/Asset/AssetSerializer.h"

#include <yaml-cpp/yaml.h>

namespace Kablunk::asset
{

	void AssetManager::init()
	{
		// register asset serializers
		// #TODO this manual process is prone to bugs since new assets must manually register their serializers
		m_asset_serializers[AssetType::Texture] = IntrusiveRef<TextureAssetSerializer>::Create();
		
		m_asset_registry.clear();
		load_asset_registry();

		// #TODO filesystem watcher

		reload_assets();

		KB_CORE_INFO("[AssetManager] Initialized!");
	}

	void AssetManager::shutdown()
	{
		write_registry_to_file();

		m_memory_assets.clear();
		m_asset_registry.clear();
		m_loaded_assets.clear();

		KB_CORE_INFO("[AssetManager] Shutdown!");
	}

	const AssetMetadata& AssetManager::get_metadata(const std::filesystem::path& filepath) const
	{
		for (const auto& [id, metadata] : m_asset_registry)
			if (metadata.filepath == filepath)
				return metadata;
#ifdef KB_EXCEPTION
		throw std::out_of_range{ "filepath not in registry" };
#endif
		return s_null_metadata;
	}

	const std::filesystem::path& AssetManager::get_relative_path(const std::filesystem::path& path) const
	{
		return std::filesystem::relative(path, Project::GetActive()->GetAssetDirectoryPath());
	}

	AssetType AssetManager::get_asset_type_from_filepath(const std::filesystem::path& path) const
	{
		return get_metadata(path).type;
	}

	const asset_id_t& AssetManager::import_asset(const std::filesystem::path& filepath)
	{
		std::filesystem::path relative_path = get_relative_path(filepath);

		const AssetMetadata& check_metadata = get_metadata(relative_path);
		if (check_metadata.is_valid())
			return check_metadata.id;

		AssetType type = get_asset_type_from_filepath(relative_path);
		if (type == AssetType::NONE)
			return uuid::nil_uuid;

		AssetMetadata metadata{
			uuid::generate(),
			type,
			relative_path,
			false, // is_memory_loaded
			false,  // is_data_loaded
		};

		m_asset_registry[metadata.id] = metadata;

		KB_CORE_INFO("[AssetManager] Imported new asset '{}'!", metadata.id);
		return metadata.id;
	}

	bool AssetManager::reload_asset_data(const asset_id_t& id)
	{
		AssetMetadata& metadata = get_metadata(id);
		if (!metadata.is_valid())
		{
			KB_CORE_ERROR("[AssetManager] Trying to reload invalid asset '{}'!", id);
			return false;
		}

		IntrusiveRef<IAsset> asset;
		metadata.is_data_loaded = try_load_asset(metadata, asset);
		if (metadata.is_data_loaded)
			m_loaded_assets[metadata.id] = asset;

		return metadata.is_data_loaded;
	}

	const asset_id_t& AssetManager::find_asset_id_based_on_filepath(const std::filesystem::path& filepath) const
	{
		return get_metadata(filepath).id;
	}

	AssetMetadata& AssetManager::get_metadata(const asset_id_t& id)
	{
		if (m_asset_registry.contains(id))
			return m_asset_registry.at(id);

		return s_null_metadata;
	}

	void AssetManager::load_asset_registry()
	{
		KB_CORE_INFO("[AssetManager] Loading asset registry!");

		// #TODO store asset registry path on project
		const std::filesystem::path& asset_registry_path = Project::GetAssetDirectoryPath() / s_asset_registry_path;
		if (!FileSystem::file_exists(asset_registry_path))
		{
			KB_CORE_ERROR("[AssetManager] Tried to load asset registry but file does not exist!");
			return;
		}

		std::ifstream stream{ asset_registry_path };
		KB_CORE_ASSERT(stream, "stream failed to load!");
		
		std::stringstream ss;
		ss << stream.rdbuf();

		YAML::Node data = YAML::Load(ss.str());
		auto asset_handles = data["Assets"];
		if (!asset_handles)
		{
			KB_CORE_ERROR("[AssetManager] Asset registry seems corrupted!");
			KB_CORE_ASSERT(false, "failure");
			return;
		}

		for (auto entry : asset_handles)
		{
			std::string filepath = entry["filepath"].as<std::string>();

			AssetMetadata metadata{
				entry["id"].as<uint64_t>(),
				static_cast<AssetType>(string_to_asset_type(entry["type"].as<std::string>())),
				filepath,
				false, // is_memory_loaded
				false,  // is_data_loaded
			};

			if (metadata.type == AssetType::NONE)
				continue;

			if (metadata.type != get_asset_type_from_filepath(filepath))
			{
				KB_CORE_WARN("[AssetManager] mismatch between stored type in asset registry and extension type!");
				metadata.type = get_asset_type_from_filepath(filepath);
			}

			// make sure asset exists in project
			if (!FileSystem::file_exists(filepath))
			{
				KB_CORE_WARN("[AssetManager] asset '{0}' not found in project, attempting to locate!");

				KB_CORE_ASSERT(false, "not implemented!");
			}

			if (metadata.id == uuid::nil_uuid)
			{
				KB_CORE_WARN("[AssetManager] Asset '{}' has null asset id!", filepath);
				continue;
			}

			KB_CORE_INFO("[AssetManager] Loaded {} assets!", m_asset_registry.size());
		}
	}

	void AssetManager::write_registry_to_file() const
	{
		struct asset_registry_entry_t
		{
			std::string filepath;
			AssetType type;
		};

		// sort asset registry by id
		std::map<asset_id_t, asset_registry_entry_t> sorted_entries;
		for (auto& [filepath, metadata] : m_asset_registry)
		{
			if (!FileSystem::file_exists(get_absolute_path(metadata)))
				continue;

			// don't save assets that only exist in memory
			if (metadata.is_memory_loaded)
				continue;

			std::string serialize_path = metadata.filepath.string();
			std::replace(serialize_path.begin(), serialize_path.end(), '\\', '/');
			sorted_entries[metadata.id] = asset_registry_entry_t{ serialize_path, metadata.type };
		}

		KB_CORE_INFO("[AssetManager] Saving '{}' assets to registry file!", sorted_entries.size());

		// write asset registry data in yaml
		YAML::Emitter out;
		out << YAML::BeginMap;

		out << YAML::Key << "Assets" << YAML::BeginSeq;
		for (auto& [id, entry] : sorted_entries)
		{
			out << YAML::BeginMap;
			out << YAML::Key << "id" << YAML::Value << id;
			out << YAML::Key << "filepath" << YAML::Value << entry.filepath;
			out << YAML::Key << "type" << YAML::Value << asset_type_to_string(entry.type);
			out << YAML::EndMap;
		}

		out << YAML::EndSeq;
		out << YAML::EndMap;

		// #TODO filesystem stuff

		// write asset registry data to file
		// #TODO get asset registry path from project
		std::filesystem::path asset_registry_path = Project::GetActive()->GetAssetDirectoryPath() / s_asset_registry_path;
		std::ofstream fout{ asset_registry_path };
		fout << out.c_str();
	}

	void AssetManager::reload_assets()
	{
		process_directory(Project::GetActive()->GetAssetDirectoryPath());
		write_registry_to_file();
	}

	void AssetManager::process_directory(const std::filesystem::path& directory_path)
	{
		// recursively process directories, starting from root (asset dir)
		for (auto entry : std::filesystem::directory_iterator(directory_path))
		{
			if (entry.is_directory())
				process_directory(entry.path());
			else
				import_asset(entry.path());
		}
	}

	void AssetManager::on_asset_renamed(const asset_id_t& id, const std::filesystem::path& new_filepath)
	{
		AssetMetadata& metadata = get_metadata(id);
		if (!metadata.is_valid())
			return;

		metadata.filepath = new_filepath;
		write_registry_to_file();
	}

	void AssetManager::on_asset_deleted(const asset_id_t& id)
	{
		AssetMetadata& metadata = get_metadata(id);
		if (!metadata.is_valid())
			return;

		m_asset_registry.remove(id);
		m_loaded_assets.erase(id);
		write_registry_to_file();
	}

	bool AssetManager::file_exists(const AssetMetadata& asset_metadata) const
	{
		KB_CORE_ASSERT(Project::GetActive(), "no active project!");
		// #TODO check if the path is relative?
		return FileSystem::file_exists(Project::GetActive()->GetAssetDirectoryPath() / asset_metadata.filepath);
	}

	void AssetManager::serialize_asset(const AssetMetadata& metadata, IntrusiveRef<IAsset>& asset) const
	{
		auto it = m_asset_serializers.find(asset->get_static_type());
		if (it != m_asset_serializers.end())
		{
			// serialize asset using found serializer
			it->second->serialize(metadata, asset);
		}
		else
			KB_CORE_ASSERT(false, "asset serializer for type asset '{}', '{}' not found!", metadata.id, asset_type_to_string(metadata.type));
	}

	bool AssetManager::try_load_asset(const AssetMetadata& metadata, IntrusiveRef<IAsset>& asset) const
	{
		auto it = m_asset_serializers.find(asset->get_static_type());
		if (it != m_asset_serializers.end())
		{
			// try loading asset using found serializer
			return it->second->try_load_data(metadata, asset);
		}
		else
			KB_CORE_ASSERT(false, "asset serializer for type asset '{}', '{}' not found!", metadata.id, asset_type_to_string(metadata.type));

		return false;
	}

	bool AssetManager::asset_exists(const std::filesystem::path& filepath) const
	{
		const AssetMetadata& metadata = get_metadata(find_asset_id_based_on_filepath(filepath));
		if (!metadata.is_valid())
			return false;

		return metadata.is_data_loaded;
	}

}
