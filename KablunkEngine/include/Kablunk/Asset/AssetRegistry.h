#pragma once

#ifndef KABLUNK_ASSET_ASSET_REGISTRY_H
#define KABLUNK_ASSET_ASSET_REGISTRY_H

#include "Kablunk/Asset/AssetMetadata.h"
#include "Kablunk/Asset/AssetTypeDefs.h"

#include <unordered_map>

namespace kb::asset
{
	class AssetRegistry
	{
	public:
		explicit AssetRegistry() noexcept = default;
		explicit AssetRegistry(const AssetRegistry&) noexcept = default;
		explicit AssetRegistry(AssetRegistry&&) noexcept = default;

		// indexing operator
		AssetMetadata& operator[](const asset_id_t& id) { return m_registry[id]; }
		// return a reference to the metadata at the id
		AssetMetadata& at(const asset_id_t& id) { return m_registry.at(id); }
		// return an immutable reference to the metadata at the id
		const AssetMetadata& at(const asset_id_t& id) const { return m_registry.at(id); }
		// return the number of elements in the registry
		size_t size() const { return m_registry.size(); }
		// check whether an id is inside the registry
		bool contains(const asset_id_t& id) const { return m_registry.find(id) != m_registry.end(); }
		// remove metadata from the registry by id
		size_t remove(const asset_id_t& id) { return m_registry.erase(id); }
		// clear the entire registry
		void clear() { m_registry.clear(); }
		// check whether the registry is empty
		bool empty() { return m_registry.empty(); }
		// return an iterator to the beginning of the registry
		auto begin() { return m_registry.begin(); }
		// return an iterator to the end of the registry
		auto end() { return m_registry.end(); }
		// return a constant iterator to the beginning of the registry
		auto begin() const { return m_registry.cbegin(); }
		// return a constant iterator to the end of the registry
		auto end() const { return m_registry.cend(); }

		AssetRegistry& operator=(const AssetRegistry&) = default;
		AssetRegistry& operator=(AssetRegistry&&) = default;

		void serialize(const std::filesystem::path& filepath) const;
		void serialize_binary(const std::filesystem::path& filepath) const;

		bool deserialize(const std::filesystem::path& filepath);
		bool deserialize_binary(const std::filesystem::path& filepath);
	private:
		// map of asset ids and metadata
		// #TODO replace with database
		kb::unordered_flat_map<asset_id_t, AssetMetadata> m_registry;
	};

}

#endif
