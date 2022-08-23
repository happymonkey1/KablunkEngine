#include "kablunkpch.h"

#include "Kablunk/Asset/AssetManager.h"

namespace Kablunk::asset
{

	void AssetManager::init()
	{

	}

	void AssetManager::shutdown()
	{

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
		KB_CORE_ASSERT(false, "not implemented!");
		return {};
	}

	AssetType AssetManager::get_asset_type_from_filepath(const std::filesystem::path& path) const
	{
		return get_metadata(path).type;
	}

	const asset_id_t& AssetManager::import_asset(const std::filesystem::path& filepath)
	{
		KB_CORE_ASSERT(false, "not implemented");
		return null_asset_id;
	}

	bool AssetManager::reload_asset_data(const asset_id_t& id)
	{
		KB_CORE_ASSERT(false, "not implemented");
		return false;
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

	}

	void AssetManager::write_registry_to_file() const
	{

	}

	void AssetManager::reload_assets()
	{

	}

	void AssetManager::process_directory(const std::filesystem::path& directory_path)
	{

	}

	void AssetManager::on_asset_renamed(const asset_id_t& id, const std::filesystem::path& new_filepath)
	{

	}

	void AssetManager::on_asset_deleted(const asset_id_t& id)
	{

	}

}
