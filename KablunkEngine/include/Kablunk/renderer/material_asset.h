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
class material_asset : public RefCounted
{
public:
	material_asset();
	material_asset(const arc<backend::material>& material);
    ~material_asset() override = default;

    static auto create() noexcept -> arc<material_asset>;
    static auto create(const arc<backend::material>& p_material) noexcept -> arc<material_asset>;

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

	auto get_material() const noexcept -> const arc<backend::material>& { return m_material; }
    auto get_material() noexcept -> arc<backend::material>& { return m_material; }
private:
	arc<backend::material> m_material;
};

class material_table : public RefCounted
{
public:
	material_table(uint32_t material_count = 1);
	material_table(arc<material_table> other);
	~material_table() = default;

	bool HasMaterial(uint32_t material_index) const { return m_materials.find(material_index) != m_materials.end(); }
	void SetMaterial(uint32_t index, arc<material_asset> material);
	void ClearMaterial(uint32_t index);

	const arc<material_asset>& GetMaterial(uint32_t index) const
	{
		KB_CORE_ASSERT(HasMaterial(index), "Material not found in map!");
		return m_materials.at(index);
	}

	std::map<uint32_t, arc<material_asset>>& GetMaterials() { return m_materials; }
	const std::map<uint32_t, arc<material_asset>>& GetMaterials() const { return m_materials; }

	uint32_t get_material_count() const { return m_material_count; }
	void SetMaterialCount(uint32_t new_count) { m_material_count = new_count; }

	void Clear();
private:
	std::map<uint32_t, arc<material_asset>> m_materials;
	uint32_t m_material_count;
};

} // end namespace kb::render

#endif
