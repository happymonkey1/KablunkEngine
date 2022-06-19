#ifndef KABLUNK_RENDERER_MATERIAL_ASSET_H
#define KABLUNK_RENDERER_MATERIAL_ASSET_H

#include "Kablunk/Core/Core.h"

#include "Kablunk/Renderer/Texture.h"
#include "Kablunk/Renderer/Material.h"

#include <map>
#include <glm/glm.hpp>

namespace Kablunk
{
	class MaterialAsset : public RefCounted
	{
	public:
		MaterialAsset();
		MaterialAsset(IntrusiveRef<Material> material);
		~MaterialAsset();

		glm::vec3& GetAlbedoColor();
		void SetAlbedoColor(const glm::vec3& albedo);

		float& GetMetalness();
		void SetMetalness(float metalness);

		float& GetRoughness();
		void SetRoughness(float roughness);

		float GetEmission();
		void SetEmission(float emission);

		IntrusiveRef<Texture2D> GetAlbedoMap();
		void SetAlbedoMap(IntrusiveRef<Texture2D> texture);
		void ClearAlbedoMap();

		IntrusiveRef<Texture2D> GetNormalMap();
		void SetNormalMap(IntrusiveRef<Texture2D> texture);
		bool IsUsingNormalMap();
		void SetUseNormalMap(bool use_normal);
		void ClearNormalMap();

		IntrusiveRef<Texture2D> GetMetalnessMap();
		void SetMetalnessMap(IntrusiveRef<Texture2D> texture);
		void ClearMetalnessMap();

		IntrusiveRef<Texture2D> GetRoughnessMap();
		void SetRoughnessMap(IntrusiveRef<Texture2D> texture);
		void ClearRoughnessMap();

		IntrusiveRef<Material> GetMaterial() const { return m_material; }
	private:
		IntrusiveRef<Material> m_material;
	};

	class MaterialTable : public RefCounted
	{
	public:
		MaterialTable(uint32_t material_count = 1);
		MaterialTable(IntrusiveRef<MaterialTable> other);
		~MaterialTable() = default;

		bool HasMaterial(uint32_t material_index) const { return m_materials.find(material_index) != m_materials.end(); }
		void SetMaterial(uint32_t index, IntrusiveRef<MaterialAsset> material);
		void ClearMaterial(uint32_t index);

		IntrusiveRef<MaterialAsset> GetMaterial(uint32_t index) const
		{
			KB_CORE_ASSERT(HasMaterial(index), "Material not found in map!");
			return m_materials.at(index);
		}

		std::map<uint32_t, IntrusiveRef<MaterialAsset>>& GetMaterials() { return m_materials; }
		const std::map<uint32_t, IntrusiveRef<MaterialAsset>>& GetMaterials() const { return m_materials; }

		uint32_t GetMaterialCount() const { return m_material_count; }
		void SetMaterialCount(uint32_t new_count) { m_material_count = new_count; }

		void Clear();
	private:
		std::map<uint32_t, IntrusiveRef<MaterialAsset>> m_materials;
		uint32_t m_material_count;
	};
}

#endif
