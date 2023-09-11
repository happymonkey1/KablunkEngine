#pragma once

#ifndef KABLUNK_ASSET_ASSET_H
#define KABLUNK_ASSET_ASSET_H

#include "Kablunk/Asset/AssetType.h"
#include "Kablunk/Asset/AssetFlag.h"
#include "Kablunk/Asset/AssetMetadata.h"
#include "Kablunk/Asset/AssetTypeDefs.h"

namespace kb::asset
{

	// Asset interface
	class IAsset : public RefCounted
	{
	public:
		virtual ~IAsset() = default;

		// get the asset id
		virtual asset_id_t get_id() const = 0;

		// set the asset id
		virtual void set_id(const asset_id_t& id) = 0;
		
		// get the asset flag(s)
		virtual asset_flag_t get_flags() const = 0;

		// check whether a specific flag is set
		virtual bool is_flag_set(asset_flag_t) const = 0;

		// set a specific flag
		virtual void set_flag(asset_flag_t flag, bool value = true) = 0;

		// check whether the asset is valid
		virtual bool is_valid() const = 0;

		// static method to get the asset type of the class. MUST BE IMPLEMENTED BY DERIVED CLASSES!
		// #TODO SFINAE to check that it is implemented
		static AssetType get_static_type() { return AssetType::NONE; }

		virtual bool operator==(const IAsset&) const = 0;
		virtual bool operator!=(const IAsset&) const = 0;
	};

	class Asset : public IAsset 
	{
	public:
		Asset() = default;
		
		asset_id_t get_id() const override { return m_id; };

		// set the asset id
		void set_id(const asset_id_t& id) override { m_id = id; }

		asset_flag_t get_flags() const override { return m_flag; }

		bool is_flag_set(asset_flag_t flag) const override { return static_cast<u8>(m_flag) & static_cast<u8>(flag); };

		void set_flag(asset_flag_t flag, bool value = true) override
		{
			if (value)
				m_flag = static_cast<asset_flag_t>(static_cast<u8>(m_flag) | static_cast<u8>(flag));
			else
				m_flag = static_cast<asset_flag_t>(static_cast<u8>(m_flag) & ~static_cast<u8>(flag));
		}

		bool is_valid() const override
		{
			return (
				(static_cast<u8>(m_flag) & static_cast<u8>(AssetFlag::Invalid)) |
				(static_cast<u8>(m_flag) & static_cast<u8>(AssetFlag::Missing))
				) == 0;
		}

		// static method to get the asset type of the class
		static AssetType get_static_type() { return AssetType::NONE; }

		bool operator ==(const IAsset& other) const override { return m_id == other.get_id(); }
		bool operator !=(const IAsset& other) const override { return !(*this == other); }
	protected:
		// id handle for the asset
		asset_id_t m_id;
		// flags associated with asset
		asset_flag_t m_flag = asset_flag_t::NONE;
	private:

		friend class AssetManager;
	};

}

#endif
