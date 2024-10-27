#ifndef KABLUNK_RENDERER_MATERIAL_ASSET_H
#define KABLUNK_RENDERER_MATERIAL_ASSET_H

#include "Kablunk/Core/Core.h"

#include "Kablunk/Renderer/backend/texture.h"
#include "Kablunk/Renderer/backend/material.h"

#include <map>
#include <glm/glm.hpp>

namespace kb::render
{ // start namespace kb::render

// TODO: this should not be with rendering...
class MaterialAsset : public RefCounted
{
public:
	MaterialAsset();
	MaterialAsset(arc<backend::material> material);
    ~MaterialAsset() override = default;

	glm::vec3& GetAlbedoColor();
	void SetAlbedoColor(const glm::vec3& albedo);

	float& GetMetalness();
	void SetMetalness(float metalness);

	float& GetRoughness();
	void SetRoughness(float roughness);

	float GetEmission();
	void SetEmission(float emission);

	arc<backend::texture_2d> GetAlbedoMap();
	void SetAlbedoMap(arc<backend::texture_2d> texture);
	void ClearAlbedoMap();

	arc<backend::texture_2d> GetNormalMap();
	void SetNormalMap(arc<backend::texture_2d> texture);
	bool IsUsingNormalMap();
	void SetUseNormalMap(bool use_normal);
	void ClearNormalMap();

	arc<backend::texture_2d> GetMetalnessMap();
	void SetMetalnessMap(arc<backend::texture_2d> texture);
	void ClearMetalnessMap();

	arc<backend::texture_2d> GetRoughnessMap();
	void SetRoughnessMap(arc<backend::texture_2d> texture);
	void ClearRoughnessMap();

	arc<backend::material> GetMaterial() const { return m_material; }
private:
	arc<backend::material> m_material;
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

} // end namespace kb::render

#endif
