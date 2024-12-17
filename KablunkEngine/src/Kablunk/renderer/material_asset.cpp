#include "kablunkpch.h"

#include "Kablunk/Renderer/material_asset.h"

#include "Kablunk/renderer/render_command.h"
#include "Kablunk/renderer/Renderer.h"

namespace kb::render
{ // start namespace kb::render

static const std::string s_albedo_color_uniform_str = "u_MaterialUniforms.AlbedoColor";
static const std::string s_use_normal_map_uniform_str = "u_MaterialUniforms.UseNormalMap";
static const std::string s_metalness_uniform_str = "u_MaterialUniforms.Metalness";
static const std::string s_roughness_uniform_str = "u_MaterialUniforms.Roughness";
static const std::string s_emission_uniform_str = "u_MaterialUniforms.Emission";

static const std::string s_albedo_map_uniform_str = "u_AlbedoTexture";
static const std::string s_normal_map_uniform_str = "u_NormalTexture";
static const std::string s_metalness_map_uniform_str = "u_MetalnessTexture";
static const std::string s_roughness_map_uniform_str = "u_RoughnessTexture";


material_asset::material_asset()
{
	m_material = backend::material::create(get_shader_library()->get(
        shader_library::k_default_shader_name
    ));

	// Set defaults
	SetAlbedoColor(glm::vec3(0.8f));
	SetEmission(0.0f);
	SetUseNormalMap(false);
	SetMetalness(0.0f);
	SetRoughness(0.4f);

	// Maps
    // #TODO refactor application singleton reference and remove
    const auto& texture = Singleton<Renderer>::get().get_white_texture();
	SetAlbedoMap(texture);
	SetNormalMap(texture);
	SetMetalnessMap(texture);
	SetRoughnessMap(texture);
}

material_asset::material_asset(const arc<backend::material>& p_material)
{
	m_material = backend::material::copy(p_material);
}

auto material_asset::create() noexcept -> arc<material_asset>
{
    return arc<material_asset>::Create();
}

auto material_asset::create(const arc<backend::material>& p_material) noexcept -> arc<material_asset>
{
    return arc<material_asset>::Create(p_material);
}

glm::vec3& material_asset::GetAlbedoColor()
{
	return m_material->get_vec3(s_albedo_color_uniform_str);
}

void material_asset::SetAlbedoColor(const glm::vec3& albedo)
{
	m_material->set(s_albedo_color_uniform_str, albedo);
}

float& material_asset::GetMetalness()
{
	return m_material->get_float(s_metalness_uniform_str);
}

void material_asset::SetMetalness(float metalness)
{
	m_material->set(s_metalness_uniform_str, metalness);
}

float& material_asset::GetRoughness()
{
	return m_material->get_float(s_roughness_uniform_str);
}

void material_asset::SetRoughness(float roughness)
{
	m_material->set(s_roughness_uniform_str, roughness);
}

float material_asset::GetEmission()
{
	return m_material->get_float(s_emission_uniform_str);
}

void material_asset::SetEmission(float emission)
{
	m_material->set(s_emission_uniform_str, emission);
}

arc<backend::texture_2d> material_asset::GetAlbedoMap()
{
	return m_material->try_get_texture_2d(s_albedo_map_uniform_str);
}

void material_asset::SetAlbedoMap(arc<backend::texture_2d> texture)
{
	m_material->set(s_albedo_map_uniform_str, texture);
}

void material_asset::ClearAlbedoMap()
{
	m_material->set(s_albedo_map_uniform_str, Singleton<Renderer>::get().get_white_texture());
}

arc<backend::texture_2d> material_asset::GetNormalMap()
{
	return m_material->try_get_texture_2d(s_normal_map_uniform_str);
}

void material_asset::SetNormalMap(arc<backend::texture_2d> texture)
{
	m_material->set(s_normal_map_uniform_str, texture);
}

bool material_asset::IsUsingNormalMap()
{
	return m_material->get_bool(s_use_normal_map_uniform_str);
}

void material_asset::SetUseNormalMap(bool use_normal)
{
	m_material->set(s_use_normal_map_uniform_str, use_normal);
}

void material_asset::ClearNormalMap()
{
	m_material->set(s_normal_map_uniform_str, Singleton<Renderer>::get().get_white_texture());
}

arc<backend::texture_2d> material_asset::GetMetalnessMap()
{
	return m_material->try_get_texture_2d(s_metalness_map_uniform_str);
}

void material_asset::SetMetalnessMap(arc<backend::texture_2d> texture)
{
	m_material->set(s_metalness_map_uniform_str, texture);
}

void material_asset::ClearMetalnessMap()
{
	m_material->set(s_metalness_map_uniform_str, Singleton<Renderer>::get().get_white_texture());
}

arc<backend::texture_2d> material_asset::GetRoughnessMap()
{
	return m_material->try_get_texture_2d(s_roughness_map_uniform_str);
}

void material_asset::SetRoughnessMap(arc<backend::texture_2d> texture)
{
	m_material->set(s_roughness_map_uniform_str, texture);
}

void material_asset::ClearRoughnessMap()
{
	m_material->set(s_roughness_map_uniform_str, Singleton<Renderer>::get().get_white_texture());
}

material_table::material_table(uint32_t material_count /*= 1*/)
	: m_material_count{ material_count }
{

}

material_table::material_table(arc<material_table> other)
{
	const auto& other_materials = other->GetMaterials();
	for (auto [index, material_asset] : other_materials)
		SetMaterial(index, material_asset);
}

void material_table::SetMaterial(uint32_t index, arc<material_asset> material)
{
	m_materials[index] = std::move(material);
	if (index >= m_material_count)
		m_material_count = index + 1;
}

void material_table::ClearMaterial(uint32_t index)
{
	KB_CORE_ASSERT(HasMaterial(index), "Material not in map!");
	m_materials.erase(index);
	if (index >= m_material_count)
		m_material_count = index + 1;
}

void material_table::Clear()
{
	m_materials.clear();
}

} // end namespace kb
