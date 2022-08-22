#pragma once

#ifndef KABLUNK_ASSET_ASSET_H
#define KABLUNK_ASSET_ASSET_H

#include "Kablunk/Asset/AssetType.h"
#include "Kablunk/Asset/AssetFlag.h"
#include "Kablunk/Asset/AssetMetadata.h"
#include "Kablunk/Asset/AssetTypeDefs.h"

namespace Kablunk::asset
{

	// Asset interface
	class IAsset : public RefCounted
	{
	public:
		virtual ~IAsset() = default;

		// get the asset id
		virtual asset_id_t get_id() const = 0;
		
		// get the asset flag(s)
		virtual AssetFlag get_flags() const = 0;

		// check whether a specific flag is set
		virtual bool is_flag_set(AssetFlag) const = 0;

		// set a specific flag
		virtual void set_flag(AssetFlag flag, bool value = true) = 0;

		// check whether the asset is valid
		virtual bool is_valid() const = 0;

		// static method to get the asset type of the class. MUST BE IMPLEMENTED BY DERIVED CLASSES!
		// #TODO SFINAE to check that it is implemented
		static AssetType get_static_type() { return AssetType::NONE; }

		virtual bool operator==(const IAsset&) const = 0;
		virtual bool operator!=(const IAsset&) const = 0;
	protected:
		// internal method to set the asset id
		virtual void set_id(const asset_id_t& id) = 0;
	};

	class Asset : public IAsset 
	{
	public:
		Asset() = default;
		
		asset_id_t get_id() const override { return m_id; };

		AssetFlag get_flags() const override { return m_flag; }

		bool is_flag_set(AssetFlag flag) const override { return static_cast<u8>(m_flag) & static_cast<u8>(flag); };

		void set_flag(AssetFlag flag, bool value = true) override
		{
			if (value)
				m_flag = static_cast<AssetFlag>(static_cast<u8>(m_flag) | static_cast<u8>(flag));
			else
				m_flag = static_cast<AssetFlag>(static_cast<u8>(m_flag) & ~static_cast<u8>(flag));
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
		// internal method to set the asset id
		void set_id(const asset_id_t& id) override { m_id = id; }
	protected:
		// id handle for the asset
		asset_id_t m_id;
		// flags associated with asset
		AssetFlag m_flag = AssetFlag::NONE;
	private:

		friend class AssetManager;
	};

}

#endif
