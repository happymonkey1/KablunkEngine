#include "kablunkpch.h"

#include "Kablunk/Renderer/MaterialAsset.h"

#include "Kablunk/Renderer/RenderCommand.h"

// #TODO remove when Application singleton is refactored
#include "Kablunk/Core/Application.h"

namespace Kablunk
{

	static const std::string s_albedo_color_uniform_str = "u_MaterialUniforms.AlbedoColor";
	static const std::string s_use_normal_map_uniform_str = "u_MaterialUniforms.UseNormalMap";
	static const std::string s_metalness_uniform_str = "u_MaterialUniforms.Metalness";
	static const std::string s_roughness_uniform_str = "u_MaterialUniforms.Roughness";
	static const std::string s_emission_uniform_str = "u_MaterialUniforms.Emission";

	static const std::string s_albedo_map_uniform_str = "u_AlbedoTexture";
	static const std::string s_normal_map_uniform_str = "u_NormalTexture";
	static const std::string s_metalness_map_uniform_str = "u_MetalnessTexture";
	static const std::string s_roughness_map_uniform_str = "u_RoughnessTexture";


	MaterialAsset::MaterialAsset()
	{
		m_material = Material::Create(render::get_shader_library()->Get("Kablunk_pbr_static"));

		// Set defaults
		SetAlbedoColor(glm::vec3(0.8f));
		SetEmission(0.0f);
		SetUseNormalMap(false);
		SetMetalness(0.0f);
		SetRoughness(0.4f);

		// Maps
        // #TODO refactor application singleton reference and remove
        auto texture = Application::Get().get_renderer_2d()->get_white_texture();
		SetAlbedoMap(texture);
		SetNormalMap(texture);
		SetMetalnessMap(texture);
		SetRoughnessMap(texture);

	}

	MaterialAsset::MaterialAsset(ref<Material> material)
	{
		m_material = Material::Copy(material);
	}

	MaterialAsset::~MaterialAsset()
	{

	}

	glm::vec3& MaterialAsset::GetAlbedoColor()
	{
		return m_material->GetVec3(s_albedo_color_uniform_str);
	}

	void MaterialAsset::SetAlbedoColor(const glm::vec3& albedo)
	{
		m_material->Set(s_albedo_color_uniform_str, albedo);
	}

	float& MaterialAsset::GetMetalness()
	{
		return m_material->GetFloat(s_metalness_uniform_str);
	}

	void MaterialAsset::SetMetalness(float metalness)
	{
		m_material->Set(s_metalness_uniform_str, metalness);
	}

	float& MaterialAsset::GetRoughness()
	{
		return m_material->GetFloat(s_roughness_uniform_str);
	}

	void MaterialAsset::SetRoughness(float roughness)
	{
		m_material->Set(s_roughness_uniform_str, roughness);
	}

	float MaterialAsset::GetEmission()
	{
		return m_material->GetFloat(s_emission_uniform_str);
	}

	void MaterialAsset::SetEmission(float emission)
	{
		m_material->Set(s_emission_uniform_str, emission);
	}

	Kablunk::ref<Kablunk::Texture2D> MaterialAsset::GetAlbedoMap()
	{
		return m_material->TryGetTexture2D(s_albedo_map_uniform_str);
	}

	void MaterialAsset::SetAlbedoMap(ref<Texture2D> texture)
	{
		m_material->Set(s_albedo_map_uniform_str, texture);
	}

	void MaterialAsset::ClearAlbedoMap()
	{
		m_material->Set(s_albedo_map_uniform_str, Application::Get().get_renderer_2d()->get_white_texture());
	}

	Kablunk::ref<Kablunk::Texture2D> MaterialAsset::GetNormalMap()
	{
		return m_material->TryGetTexture2D(s_normal_map_uniform_str);
	}

	void MaterialAsset::SetNormalMap(ref<Texture2D> texture)
	{
		m_material->Set(s_normal_map_uniform_str, texture);
	}

	bool MaterialAsset::IsUsingNormalMap()
	{
		return m_material->GetBool(s_use_normal_map_uniform_str);
	}

	void MaterialAsset::SetUseNormalMap(bool use_normal)
	{
		m_material->Set(s_use_normal_map_uniform_str, use_normal);
	}

	void MaterialAsset::ClearNormalMap()
	{
		m_material->Set(s_normal_map_uniform_str, Application::Get().get_renderer_2d()->get_white_texture());
	}

	Kablunk::ref<Kablunk::Texture2D> MaterialAsset::GetMetalnessMap()
	{
		return m_material->TryGetTexture2D(s_metalness_map_uniform_str);
	}

	void MaterialAsset::SetMetalnessMap(ref<Texture2D> texture)
	{
		m_material->Set(s_metalness_map_uniform_str, texture);
	}

	void MaterialAsset::ClearMetalnessMap()
	{
		m_material->Set(s_metalness_map_uniform_str, Application::Get().get_renderer_2d()->get_white_texture());
	}

	Kablunk::ref<Kablunk::Texture2D> MaterialAsset::GetRoughnessMap()
	{
		return m_material->TryGetTexture2D(s_roughness_map_uniform_str);
	}

	void MaterialAsset::SetRoughnessMap(ref<Texture2D> texture)
	{
		m_material->Set(s_roughness_map_uniform_str, texture);
	}

	void MaterialAsset::ClearRoughnessMap()
	{
		m_material->Set(s_roughness_map_uniform_str, Application::Get().get_renderer_2d()->get_white_texture());
	}

	MaterialTable::MaterialTable(uint32_t material_count /*= 1*/)
		: m_material_count{ material_count }
	{
		
	}

	MaterialTable::MaterialTable(ref<MaterialTable> other)
	{
		const auto& other_materials = other->GetMaterials();
		for (auto [index, material_asset] : other_materials)
			SetMaterial(index, material_asset);
	}

	void MaterialTable::SetMaterial(uint32_t index, ref<MaterialAsset> material)
	{
		m_materials[index] = material;
		if (index >= m_material_count)
			m_material_count = index + 1;
	}

	void MaterialTable::ClearMaterial(uint32_t index)
	{
		KB_CORE_ASSERT(HasMaterial(index), "Material not in map!");
		m_materials.erase(index);
		if (index >= m_material_count)
			m_material_count = index + 1;
	}

	void MaterialTable::Clear()
	{
		m_materials.clear();
	}

}
