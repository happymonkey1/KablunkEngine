#ifndef KABLUNK_CORE_ASSET_MANAGER_H
#define KABLUNK_CORE_ASSET_MANAGER_H

#include "Kablunk/Core/Uuid64.h"
#include "Kablunk/Renderer/Texture.h"

#include <unordered_map>

namespace Kablunk
{
	using UUID = uuid::uuid64;


	class BaseAsset
	{
	public:
		BaseAsset() = default;
		BaseAsset(const std::string& filepath) 
			: m_uuid{ uuid::generate() }, m_filepath{ filepath }
		{ 

		}
		BaseAsset(const std::string& filepath, const UUID& uuid) 
			: m_uuid{ uuid }, m_filepath{ filepath } { }

		virtual ~BaseAsset() = default;
	
		const UUID& GetUUID() const { return m_uuid; }
		const std::string& GetFilepath() const { return m_filepath; }

	protected:
		UUID m_uuid{ };
		std::string m_filepath{ "" };
	};


	template <class Content>
	class Asset : public BaseAsset
	{
	public:
		// Should probably determine null assets a different way since UUID could potentially generate as zero
		template <class T>
		static constexpr Asset<T> null_asset{};
	public:
		Asset() = default;
		Asset(const std::string& filepath)
			: BaseAsset{ filepath } { }
		Asset(const std::string& filepath, const UUID& id)
			: BaseAsset{ filepath, id } { };
		virtual ~Asset() override = default;

		Ref<Content> Get() const { return m_asset; }
		// Returns raw pointer to content
		Content* Raw() { return m_asset.get(); }

		operator bool() const { return !uuid::is_nil(m_uuid) && m_asset.get() != nullptr; }
		bool operator==(const Asset<Content>& rhs) const { return m_uuid != rhs.m_uuid; }
		bool operator!=(const Asset<Content>& rhs) const { return !(*this == rhs); }

	private:
		Ref<Content> m_asset{ nullptr };

		friend class AssetManager;
	};

	// Asset Specializations
	// #TODO Probably a better way than having template specializations

	
	template<> 
	Asset<Texture2D>::Asset(const std::string& filepath) 
		: BaseAsset{ filepath }, m_asset{ Texture2D::Create(filepath) } { }

	template<>
	Asset<Texture2D>::Asset(const std::string& filepath, const UUID& id) 
		: BaseAsset{ filepath, id }, m_asset{ Texture2D::Create(filepath) } { }

	// =================
	// | Asset manager |
	// =================

	class BasicAssetException : public std::logic_error
	{
	public:
		BasicAssetException(const std::string& error) : std::logic_error{ error } { };
	};
	class DuplicateAssetException : public std::logic_error
	{
	public:
		DuplicateAssetException(const std::string& error) : std::logic_error{ error } { }
	};
	class UnsupportedAssetTypeException : public std::logic_error
	{
	public:
		UnsupportedAssetTypeException(const std::string& error) : std::logic_error{ error } { }
	};

	class AssetManager
	{
	public:
		template <typename T>
		static Asset<T> Create(const std::string& filepath)
		{
			return create_asset<T>(filepath);
		}

		template <typename T>
		static Asset<T> Create(const std::string& filepath, const UUID& uuid)
		{

			auto it = s_asset_store.find(uuid);
			if (it == s_asset_store.end())
				return create_asset<T>(filepath, uuid);
			else
			{
				KB_CORE_ERROR("uuid: {0} already found in asset store!", uuid);

#ifdef KB_DEBUG
				throw DuplicateAssetException{ "Trying to create an asset that already exists!" };
#else
				KB_CORE_WARN("Returning asset with uuid: {0} instead!", uuid);
#endif
				return Asset<T>{};
			}
			
		}

		template <typename T>
		static Asset<T> Get(const UUID& uuid)
		{
			auto it = s_asset_store.find(uuid);
			if (it == s_asset_store.end())
			{
				KB_CORE_ERROR("uuid: {0} not found in asset store!", uuid);
#ifdef KB_DEBUG
				throw BasicAssetException{ "Asset not found in asset store! " };
#else
				KB_CORE_WARN("Returning null asset instead!");
#endif
				return Asset<T>{};
			}
			else
			{
				return get_asset<T>(it);
			}
		}

		template <typename T>
		static Asset<T> Get(const std::string& filepath)
		{
			for (auto it : s_asset_store)
			{
				auto& asset = it->second;
				if (asset && asset->GetFilepath() == filepath)
					return get_asset<T>(it);
				
			}

			KB_CORE_ERROR("could not find file with path: '{0}' in asset store!", filepath);
#ifdef KB_DEBUG
			throw BasicAssetException{ "Asset not found in store!" };
#else
			KB_CORE_WARN("Creating new asset instead!");
#endif
			return Create<T>(filepath);
		}

	private:
		template <typename T>
		static Asset<T> create_asset(const std::string& filepath, const UUID& uuid = uuid::nil_uuid)
		{
			Asset<T> asset;
			if (!uuid::is_nil(uuid)) 
				asset = Asset<T>{ filepath, uuid };
			else
				asset = Asset<T>{ filepath };

			return asset;
		}

		template <typename T>
		static Asset<T> get_asset(std::unordered_map<UUID, BaseAsset*>::iterator it)
		{
			auto& asset = it->second;
			auto& uuid = asset->GetUUID();
			try
			{
				return dynamic_cast<T>(*asset);
			}
			catch (...)
			{
				KB_CORE_ERROR("Failed to cast type {0} while returning asset with uuid: {1}. Most likely incorrect type!",
					typeid(T).name(), uuid);
#ifdef KB_DEBUG
				throw UnsupportedAssetTypeException{ "Unsupported asset type! " };
#else
				KB_CORE_WARN("Returning null asset instead!");
#endif
				return Asset<T>{};
			}
		}


	private:
		static std::unordered_map<UUID, BaseAsset*> s_asset_store;

		template <typename T>
		friend class Asset;
	};
}

#endif
