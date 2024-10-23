#ifndef KABLUNK_RENDERER_MATERIAL_ASSET_H
#define KABLUNK_RENDERER_MATERIAL_ASSET_H

#include "Kablunk/Core/Core.h"

#include "Kablunk/Renderer/Texture.h"
#include "Kablunk/Renderer/Material.h"

#include <map>
#include <glm/glm.hpp>

namespace kb
{
	class MaterialAsset : public RefCounted
	{
	public:
		MaterialAsset();
		MaterialAsset(arc<Material> material);
        ~MaterialAsset() override = default;

		glm::vec3& GetAlbedoColor();
		void SetAlbedoColor(const glm::vec3& albedo);

		float& GetMetalness();
		void SetMetalness(float metalness);

		float& GetRoughness();
		void SetRoughness(float roughness);

		float GetEmission();
		void SetEmission(float emission);

		arc<Texture2D> GetAlbedoMap();
		void SetAlbedoMap(arc<Texture2D> texture);
		void ClearAlbedoMap();

		arc<Texture2D> GetNormalMap();
		void SetNormalMap(arc<Texture2D> texture);
		bool IsUsingNormalMap();
		void SetUseNormalMap(bool use_normal);
		void ClearNormalMap();

		arc<Texture2D> GetMetalnessMap();
		void SetMetalnessMap(arc<Texture2D> texture);
		void ClearMetalnessMap();

		arc<Texture2D> GetRoughnessMap();
		void SetRoughnessMap(arc<Texture2D> texture);
		void ClearRoughnessMap();

		arc<Material> GetMaterial() const { return m_material; }
	private:
		arc<Material> m_material;
	};

	class MaterialTable : public RefCounted
	{
	public:
		MaterialTable(uint32_t material_count = 1);
		MaterialTable(arc<MaterialTable> other);
		~MaterialTable() = default;

		bool HasMaterial(uint32_t material_index) const { return m_materials.find(material_index) != m_materials.end(); }
		void SetMaterial(uint32_t index, arc<MaterialAsset> material);
		void ClearMaterial(uint32_t index);

		const arc<MaterialAsset>& GetMaterial(uint32_t index) const
		{
			KB_CORE_ASSERT(HasMaterial(index), "Material not found in map!");
			return m_materials.at(index);
		}

		std::map<uint32_t, arc<MaterialAsset>>& GetMaterials() { return m_materials; }
		const std::map<uint32_t, arc<MaterialAsset>>& GetMaterials() const { return m_materials; }

		uint32_t GetMaterialCount() const { return m_material_count; }
		void SetMaterialCount(uint32_t new_count) { m_material_count = new_count; }

		void Clear();
	private:
		std::map<uint32_t, arc<MaterialAsset>> m_materials;
		uint32_t m_material_count;
	};
}

#endif
