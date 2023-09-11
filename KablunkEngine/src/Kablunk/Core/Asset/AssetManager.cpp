#include "kablunkpch.h"

#include "Kablunk/Asset/AssetManager.h"

#include "Kablunk/Asset/AssetSerializer.h"

#include "Kablunk/Renderer/Font/FontManager.h"
#include "Kablunk/Renderer/RenderCommand2D.h"

#include "Kablunk/Core/Application.h"

#include <yaml-cpp/yaml.h>

namespace Kablunk::asset
{

	void AssetManager::init()
	{
        KB_CORE_INFO("[asset_manager]: starting initialization");

		// register asset serializers
		// #TODO this manual process is prone to bugs since new assets must manually register their serializers
		m_asset_serializers[AssetType::Texture] = ref<TextureAssetSerializer>::Create();
		m_asset_serializers[AssetType::Audio] = ref<AudioAssetSerializer>::Create();
		m_asset_serializers[AssetType::Font] = ref<font_asset_serializer>::Create();
		
		m_asset_registry.clear();
		load_asset_registry();

		// #TODO filesystem watcher

        // load engine default assets
        // #TODO bug: roboto font is always loaded, filling asset registry with duplicates since it is not located in "asset" folder
        {
            // import default font asset metadata
            auto default_font_asset_id = import_engine_asset_metadata("fonts/roboto/Roboto-Regular.ttf");
            KB_CORE_ASSERT(default_font_asset_id != asset::null_asset_id, "[asset_manager]: tried loading default font, but asset_import returned null id?");

            ref<render::font_asset_t> default_font_asset = get_asset<render::font_asset_t>(default_font_asset_id);

            // load into font registry
            Application::Get().get_renderer_2d()->get_font_manager().add_font_file_to_library(default_font_asset);
            //render2d::get_font_manager().add_font_file_to_library(default_font_asset);
        }

		reload_assets();

		KB_CORE_INFO("[asset_manager] finished initialization!");
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
        auto relative_path = get_relative_path(filepath);

		for (const auto& [id, metadata] : m_asset_registry)
			if (metadata.filepath == relative_path)
				return metadata;
#ifdef KB_EXCEPTION
		throw std::out_of_range{ "filepath not in registry" };
#endif
		return s_null_metadata;
	}

    // get a relative path for a given path stored on some asset metadata
    std::filesystem::path AssetManager::get_relative_path(const AssetMetadata& p_metadata) const
    {
        const bool is_internal_asset = p_metadata.is_internal_asset;

        if (p_metadata.filepath.is_relative())
            return p_metadata.filepath;

        auto relative_to_path = is_internal_asset ? Singleton<ProjectManager>::get().get_active()->get_project_directory() : 
            Singleton<ProjectManager>::get().get_active()->get_project_directory();

        return std::filesystem::relative(p_metadata.filepath, relative_to_path);
    }

	std::filesystem::path AssetManager::get_relative_path(const std::filesystem::path& path) const
	{
		return path.is_relative() ? path : std::filesystem::relative(path, ProjectManager::get().get_active()->get_asset_directory_path());
	}

    

	AssetType AssetManager::get_asset_type_from_filepath(const std::filesystem::path& path) const
	{
		// try use asset metadata if available
		return get_metadata(path).is_valid() ? get_metadata(path).type : get_asset_type_by_extension(path.extension().string());
	}

	asset_id_t AssetManager::import_asset(const std::filesystem::path& filepath)
	{
		std::filesystem::path relative_path = get_relative_path(filepath);
		if (relative_path.empty())
		{
			KB_CORE_WARN("[AssetManager] Path '{}' relative to '{}' is empty!", filepath, ProjectManager::get().get_active()->get_asset_directory_path());
			return asset::null_asset_id;
		}

		if (is_asset_registry_file(relative_path))
		{
			KB_CORE_WARN("[AssetManager] Trying to import asset registry file as an asset!");
			return asset::null_asset_id;
		}

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
            false,  // internal engine asset
		};

		// #TODO why do we not try to load data?

		m_asset_registry[metadata.id] = metadata;

		KB_CORE_INFO("[AssetManager] Imported new asset '{}'!", metadata.id);
		return metadata.id;
	}

	bool AssetManager::reload_asset_data(const asset_id_t& id)
	{
		AssetMetadata& metadata = get_metadata_internal(id);
		if (!metadata.is_valid())
		{
			KB_CORE_ERROR("[AssetManager] Trying to reload invalid asset '{}'!", id);
			return false;
		}

		ref<IAsset> asset;
		metadata.is_data_loaded = try_load_asset(metadata, asset);
		if (metadata.is_data_loaded)
			m_loaded_assets[metadata.id] = asset;

		return metadata.is_data_loaded;
	}

	const asset_id_t& AssetManager::find_asset_id_based_on_filepath(const std::filesystem::path& filepath) const
	{
		return get_metadata(filepath).id;
	}

	AssetMetadata& AssetManager::get_metadata_internal(const asset_id_t& id)
	{
		if (m_asset_registry.contains(id))
			return m_asset_registry.at(id);

		return s_null_metadata;
	}

	void AssetManager::load_asset_registry()
	{
		KB_CORE_INFO("[AssetManager] Loading asset registry!");

		// #TODO store asset registry path on project
		const std::filesystem::path& asset_registry_path = ProjectManager::get().get_active()->get_asset_directory_path() / s_asset_registry_path;
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

            const bool is_internal_asset = entry["is_internal_asset"] ? entry["is_internal_asset"].as<bool>() : false;

			AssetMetadata metadata{
				entry["id"].as<uint64_t>(),
				static_cast<AssetType>(string_to_asset_type(entry["type"].as<std::string>())),
				filepath,
				false, // is_memory_loaded
				false,  // is_data_loaded
                is_internal_asset
			};

			if (metadata.type == AssetType::NONE)
			{
				KB_CORE_WARN("[AssetManager] AssetType NONE for asset '{}' encountered while loading asset registry file!", metadata.id);
				continue;
			}

			if (metadata.type != get_asset_type_from_filepath(filepath))
			{
				KB_CORE_WARN("[AssetManager] Mismatch between stored type in asset registry and extension type!");
				metadata.type = get_asset_type_from_filepath(filepath);
			}

			// make sure asset exists in project
			if (!FileSystem::file_exists(get_absolute_path(metadata)))
			{
				KB_CORE_WARN("[AssetManager] Asset '{0}' not found in project, attempting to locate on disk!", get_absolute_path(metadata));

				KB_CORE_ASSERT(false, "not implemented!");
			}

			if (metadata.id == uuid::nil_uuid)
			{
				KB_CORE_WARN("[AssetManager] Asset '{}' has null asset id!", filepath);
				continue;
			}

            m_asset_registry[metadata.id] = metadata;

			KB_CORE_INFO("[AssetManager] Loaded {} assets!", m_asset_registry.size());
		}
	}

	void AssetManager::write_registry_to_file() const
	{
		struct asset_registry_entry_t
		{
			std::string m_filepath = "";
			AssetType m_type = AssetType::NONE;
            bool m_is_internal_asset = false;
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
			sorted_entries[metadata.id] = asset_registry_entry_t{ serialize_path, metadata.type, metadata.is_internal_asset };
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
			out << YAML::Key << "filepath" << YAML::Value << entry.m_filepath;
			out << YAML::Key << "type" << YAML::Value << asset_type_to_string(entry.m_type);
            out << YAML::Key << "is_internal_asset" << YAML::Value << entry.m_is_internal_asset;
			out << YAML::EndMap;
		}

		out << YAML::EndSeq;
		out << YAML::EndMap;

		// #TODO filesystem stuff

		// write asset registry data to file
		// #TODO get asset registry path from project
		std::filesystem::path asset_registry_path = ProjectManager::get().get_active()->get_asset_directory_path() / s_asset_registry_path;
		std::ofstream fout{ asset_registry_path };
		fout << out.c_str();
	}

	void AssetManager::reload_assets()
	{
		process_directory(ProjectManager::get().get_active()->get_asset_directory_path());
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
		AssetMetadata& metadata = get_metadata_internal(id);
		if (!metadata.is_valid())
			return;

		metadata.filepath = new_filepath;
		write_registry_to_file();
	}

	void AssetManager::on_asset_deleted(const asset_id_t& id)
	{
		AssetMetadata& metadata = get_metadata_internal(id);
		if (!metadata.is_valid())
			return;

		m_asset_registry.remove(id);
		m_loaded_assets.erase(id);
		write_registry_to_file();
	}

	bool AssetManager::file_exists(const AssetMetadata& asset_metadata) const
	{
		KB_CORE_ASSERT(ProjectManager::get().get_active(), "no active project!");
		// #TODO check if the path is relative?
		return FileSystem::file_exists(ProjectManager::get().get_active()->get_asset_directory_path() / asset_metadata.filepath);
	}

	void AssetManager::serialize_asset(const AssetMetadata& metadata, ref<IAsset>& asset) const
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

	bool AssetManager::try_load_asset(const AssetMetadata& metadata, ref<IAsset>& asset) const
	{
		auto it = m_asset_serializers.find(metadata.type);
		if (it != m_asset_serializers.end())
		{
			KB_CORE_INFO("[AssetManager] Trying to load '{}' from disk using serializer={}", metadata.filepath, asset_type_to_string(metadata.type));
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

    asset::asset_id_t AssetManager::import_engine_asset_metadata(const std::filesystem::path& filepath)
    {
        auto absolute_path = Singleton<ProjectManager>::get().get_active()->get_project_directory() / "resources" / filepath;

        KB_CORE_ASSERT(std::filesystem::exists(absolute_path), "[asset_manager]: trying to load internal engine asset that does not exist?");

        if (is_asset_registry_file(absolute_path))
        {
            KB_CORE_WARN("[AssetManager] Trying to import asset registry file as an asset!");
            return asset::null_asset_id;
        }

        const AssetMetadata& check_metadata = get_metadata(absolute_path);
        if (check_metadata.is_valid())
            return check_metadata.id;

        AssetType type = get_asset_type_from_filepath(absolute_path);
        if (type == AssetType::NONE)
            return uuid::nil_uuid;

        AssetMetadata metadata{
            uuid::generate(),
            type,
            filepath,
            false, // is_memory_loaded
            false,  // is_data_loaded
            true        // internal engine asset
        };

        // #TODO why do we not try to load data?

        m_asset_registry[metadata.id] = metadata;

        KB_CORE_INFO("[AssetManager] Imported new asset '{}'!", metadata.id);
        return metadata.id;
    }

}
