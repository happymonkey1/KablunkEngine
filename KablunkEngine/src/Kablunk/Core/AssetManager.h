#ifndef KABLUNK_CORE_ASSET_MANAGER_H
#define KABLUNK_CORE_ASSET_MANAGER_H

#include "Kablunk/Renderer/Texture.h"

#include <unordered_map>

namespace Kablunk
{
	using UUID = uint64_t;


	class BaseAsset
	{
	public:
		BaseAsset() = default;
		virtual ~BaseAsset() = default;
	
		UUID GetUUID() const { return m_uuid; }
		const std::string& GetFilepath() const { return m_filepath; }

	protected:
		UUID m_uuid{ 0 };
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
			: m_uuid{ 0 }, m_filepath{ filepath }
		{
			KB_CORE_WARN("UUID generator not used for asset uuid, initialized to 0 instead!");
		}
		virtual ~Asset() override = default;

		Ref<Content> Get() const { return m_asset; }
		// Returns raw pointer to content
		Content* Raw() { return m_asset.get(); }

		operator bool() const { return m_uuid != null_asset && m_asset.get() != nullptr; }
		bool operator==(const Asset<Content>& rhs) const { return m_uuid != rhs.m_uuid; }
		bool operator!=(const Asset<Content>& rhs) const { return !(*this == rhs); }

	private:
		Ref<Content> m_asset{ CreateRef<Content>{ nullptr } };

		friend class AssetManager;
	};

	// =================
	// | Asset manager |
	// =================

	class AssetManager
	{
	public:
		template <typename T>
		static void Create(const std::string& filepath)
		{
			switch (typeid(T))
			{
				case typeid(Texture):
				{
					auto asset = Asset<T>{ filepath };
					asset.m_asset = Texture::Create(filepath);
					m_asset_store.emplace({ asset.GetUUID(), &asset })
					return asset;
				}
				default:
				{
					KB_CORE_ERROR("AssetManager does not support type: {0}", typeid(T).name());
					KB_CORE_WARN("Returning null asset instead!");
					return Asset<T>{};
				}
			}
		}

		template <typename T>
		static Asset<T> Get(const UUID& uuid)
		{
			auto it = m_asset_store.find(uuid);
			if (it == m_asset_store.end())
			{
				KB_CORE_ERROR("uuid: {0} not found in asset store!");
				KB_CORE_WARN("Returning null asset instead!");
				return Asset<T>{};
			}
			else
			{
				try
				{
					return dynamic_cast<T>(*it);
				}
				catch (...)
				{
					KB_CORE_ERROR("Failed to cast type {0} while returning asset with uuid: {1}. Most likely incorrect type!",
						typeid(T).name(), uuid);
					KB_CORE_WARN("Returning null asset instead!");
					return Asset<T>{};
				}
			}
		}

		template <typename T>
		static Asset<T> Get(const std::string& filepath)
		{
			for (auto it : m_asset_store)
			{
				auto& asset = it->second;
				if (asset)
				{
					if (asset->GetFilepath() != filepath) continue;
					else
					{
						auto uuid = asset->GetUUID();
						try
						{
							return dynamic_cast<T>(*asset);
						}
						catch (...)
						{
							KB_CORE_ERROR("Failed to cast type {0} while returning asset with uuid: {1}. Most likely incorrect type!",
								typeid(T).name(), uuid);
							KB_CORE_WARN("Returning null asset instead!");
							return Asset<T>{};
						}
					}
				}
			}

			KB_CORE_ERROR("could not find file with path: '{0}' in asset store!", filepath);
			KB_CORE_WARN("Creating new asset instead!");
			return Create<T>(filepath);
		}

	private:
		std::unordered_map<UUID, BaseAsset*> m_asset_store;

		template <typename T>
		friend class Asset;
	};
}

#endif
