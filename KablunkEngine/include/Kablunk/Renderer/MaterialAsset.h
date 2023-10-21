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
		MaterialAsset(ref<Material> material);
		~MaterialAsset();

		glm::vec3& GetAlbedoColor();
		void SetAlbedoColor(const glm::vec3& albedo);

		float& GetMetalness();
		void SetMetalness(float metalness);

		float& GetRoughness();
		void SetRoughness(float roughness);

		float GetEmission();
		void SetEmission(float emission);

		ref<Texture2D> GetAlbedoMap();
		void SetAlbedoMap(ref<Texture2D> texture);
		void ClearAlbedoMap();

		ref<Texture2D> GetNormalMap();
		void SetNormalMap(ref<Texture2D> texture);
		bool IsUsingNormalMap();
		void SetUseNormalMap(bool use_normal);
		void ClearNormalMap();

		ref<Texture2D> GetMetalnessMap();
		void SetMetalnessMap(ref<Texture2D> texture);
		void ClearMetalnessMap();

		ref<Texture2D> GetRoughnessMap();
		void SetRoughnessMap(ref<Texture2D> texture);
		void ClearRoughnessMap();

		ref<Material> GetMaterial() const { return m_material; }
	private:
		ref<Material> m_material;
	};

	class MaterialTable : public RefCounted
	{
	public:
		MaterialTable(uint32_t material_count = 1);
		MaterialTable(ref<MaterialTable> other);
		~MaterialTable() = default;

		bool HasMaterial(uint32_t material_index) const { return m_materials.find(material_index) != m_materials.end(); }
		void SetMaterial(uint32_t index, ref<MaterialAsset> material);
		void ClearMaterial(uint32_t index);

		const ref<MaterialAsset>& GetMaterial(uint32_t index) const
		{
			KB_CORE_ASSERT(HasMaterial(index), "Material not found in map!");
			return m_materials.at(index);
		}

		std::map<uint32_t, ref<MaterialAsset>>& GetMaterials() { return m_materials; }
		const std::map<uint32_t, ref<MaterialAsset>>& GetMaterials() const { return m_materials; }

		uint32_t GetMaterialCount() const { return m_material_count; }
		void SetMaterialCount(uint32_t new_count) { m_material_count = new_count; }

		void Clear();
	private:
		std::map<uint32_t, ref<MaterialAsset>> m_materials;
		uint32_t m_material_count;
	};
}

#endif
