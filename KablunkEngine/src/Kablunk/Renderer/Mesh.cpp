#include "kablunkpch.h"

#include "Kablunk/Core/Core.h"
#include "Kablunk/Renderer/Mesh.h"
#include "Kablunk/renderer/render_command.h"
#include "Kablunk/Scene/Entity.h"

#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>

#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

// #TODO refactor Application singleton reference and remove
#include "Kablunk/Core/Application.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>

#include <filesystem>

namespace kb::render
{ // start namespace kb::render

static constexpr u32 s_mesh_import_flags =
	aiProcess_Triangulate | aiProcess_GenNormals | aiProcess_CalcTangentSpace | aiProcess_GenUVCoords | aiProcess_ValidateDataStructure;

namespace util
{
	glm::mat4 Mat4FromAssimpMat4(const aiMatrix4x4& matrix)
	{
		glm::mat4 result;
		//the a,b,c,d in assimp is the row ; the 1,2,3,4 is the column
		result[0][0] = matrix.a1; result[1][0] = matrix.a2; result[2][0] = matrix.a3; result[3][0] = matrix.a4;
		result[0][1] = matrix.b1; result[1][1] = matrix.b2; result[2][1] = matrix.b3; result[3][1] = matrix.b4;
		result[0][2] = matrix.c1; result[1][2] = matrix.c2; result[2][2] = matrix.c3; result[3][2] = matrix.c4;
		result[0][3] = matrix.d1; result[1][3] = matrix.d2; result[2][3] = matrix.d3; result[3][3] = matrix.d4;
		return result;
	}

    auto init_material_table(
        arc<material_table>& p_material_table,
        const std::vector<arc<material_asset>>& p_materials
    ) noexcept -> void
    {
        p_material_table = arc<material_table>::Create(p_materials.size());
        for (size_t i = 0; i < p_materials.size(); ++i)
        {
            p_material_table->SetMaterial(static_cast<u32>(i), p_materials[i]);
        }
    }
}

MeshData::MeshData(const std::string& filepath, Entity entity)
	: m_filepath{ filepath }, m_handle{ mesh_handle::into(std::string_view{ filepath }) }
{
	KB_CORE_TRACE("Loading mesh: '{0}'", filepath.c_str());

	m_importer = create_box<Assimp::Importer>();
    m_importer->SetPropertyBool(AI_CONFIG_IMPORT_FBX_PRESERVE_PIVOTS, false);

	const aiScene* scene = m_importer->ReadFile(m_filepath, s_mesh_import_flags);
	if (!scene || !scene->HasMeshes())
	{
		KB_CORE_ERROR("Failed to load mesh file: '{0}'", m_filepath);
		return;
	}

	m_is_animated = scene->mAnimations != nullptr;
	if (m_is_animated)
		KB_CORE_INFO("ANIMATED MESH!");

    const auto renderer_pipeline = get_renderer_pipeline_type();
    switch (renderer_pipeline)
    {
    case renderer_pipeline_type_t::pbr:
    {
        KB_CORE_ASSERT(!m_is_animated, "[mesh]: Cannot set PBR shader for animated mesh, shader does not exist!");
        m_mesh_shader = render::get_shader(shader_library::k_pbr_static_shader_name);
        break;
    }
    case renderer_pipeline_type_t::basic:
    {
        m_mesh_shader = m_is_animated ? render::get_shader(shader_library::k_diffuse_anim_shader_name) : render::get_shader(shader_library::k_diffuse_static_shader_name);
        break;
    }
    default:
        KB_CORE_ASSERT(false, "[mesh]: Unable to determine mesh shader for unknown renderer pipeline!");
    }

	m_inverse_transform = glm::inverse(util::Mat4FromAssimpMat4(scene->mRootNode->mTransformation));

	size_t vertex_count = 0;
	size_t index_count = 0;

    constexpr bool k_use_submesh_batching = true;

	m_sub_meshes.reserve(scene->mNumMeshes);
	for (size_t ai_mesh_index = 0; ai_mesh_index < scene->mNumMeshes; ++ai_mesh_index)
	{
		aiMesh* mesh = scene->mMeshes[ai_mesh_index];

        if constexpr (k_use_submesh_batching)
        {
            sub_mesh_t& sub_mesh = m_sub_meshes.emplace_back();
            sub_mesh.BaseVertex = static_cast<u32>(vertex_count);
            sub_mesh.BaseIndex = static_cast<u32>(index_count);
            sub_mesh.Material_index = mesh->mMaterialIndex;
            // KB_CORE_ASSERT(mesh->mMaterialIndex > 0, "[mesh]: Material_index={} out of bounds!", sub_mesh.Material_index);
            sub_mesh.VertexCount = mesh->mNumVertices;
            sub_mesh.IndexCount = mesh->mNumFaces * 3;
            sub_mesh.mesh_name = mesh->mName.C_Str();

            vertex_count += mesh->mNumVertices;
            index_count += sub_mesh.IndexCount;
        }
		

		if (m_is_animated)
		{
			for (size_t i = 0; i < mesh->mNumVertices; ++i)
			{
				animated_vertex_t v;
				v.Position = vec3_packed{ mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z };
				v.Normal = vec3_packed{ mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z };
				v.EntityID = static_cast<int32_t>(entity);

				if (mesh->HasTangentsAndBitangents())
				{
					v.Tangent = vec3_packed{ mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z };
					v.Binormal = vec3_packed{ mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z };
				}

				if (mesh->HasTextureCoords(0))
				{
					// #FIXME figure out how to deal with multiple textures
					v.TexCoord = vec2_packed{ mesh->mTextureCoords[0]->x, mesh->mTextureCoords[0]->y };
				}

				m_animated_vertices.push_back(v);
			}
		}
		else
		{
			for (size_t i = 0; i < mesh->mNumVertices; ++i)
			{
				vertex_t v;
				v.Position = vec3_packed{ mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z };
				v.Normal = vec3_packed{ mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z };

				if (mesh->HasTangentsAndBitangents())
				{
					v.Tangent = vec3_packed{ mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z };
					v.Binormal = vec3_packed{ mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z };
				}

				if (mesh->HasTextureCoords(0))
				{
					// #FIXME figure out how to deal with multiple textures
					v.TexCoord = vec2_packed{ mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y };
				}

				m_static_vertices.push_back(v);
			}
		}


		for (u32 i = 0; i < mesh->mNumFaces; ++i)
		{
			KB_CORE_ASSERT(mesh->mFaces[i].mNumIndices == 3, "Must have 3 indices");
			Index index = { mesh->mFaces[i].mIndices[0], mesh->mFaces[i].mIndices[1], mesh->mFaces[i].mIndices[2] };
			m_indices.push_back(index);

			if (!m_is_animated)
			{
                const auto& sub_mesh = m_sub_meshes[ai_mesh_index];
				m_triangle_cache[static_cast<u32>(ai_mesh_index)].emplace_back(
					m_static_vertices[index.V1 + sub_mesh.BaseVertex],
					m_static_vertices[index.V2 + sub_mesh.BaseVertex],
					m_static_vertices[index.V3 + sub_mesh.BaseVertex]
				);
                m_triangle_count++;
			}
		}
	}

	TraverseNodes(scene->mRootNode);

	// Bones
	if (m_is_animated)
	{
		for (size_t i = 0; i < scene->mNumMeshes; ++i)
		{
			aiMesh* mesh = scene->mMeshes[i];
			sub_mesh_t& submesh = m_sub_meshes[i];

			for (size_t b = 0; b < mesh->mNumBones; ++b)
			{
				aiBone* bone = mesh->mBones[b];
				std::string bone_name = bone->mName.data;
				uint32_t bone_index = 0;

				if (m_bone_mapping.find(bone_name) == m_bone_mapping.end())
				{
					bone_index = m_bone_count++;
					bone_info_t& bone_info = m_bone_info.emplace_back();
					bone_info.Bone_offset = util::Mat4FromAssimpMat4(bone->mOffsetMatrix);
					m_bone_mapping[bone_name] = bone_index;
				}
				else
				{
					KB_CORE_WARN("Found existing bone in map");
					bone_index = m_bone_mapping[bone_name];
				}

				for (size_t j = 0; j < bone->mNumWeights; ++j)
				{
					uint32_t vertex_id = submesh.BaseVertex + bone->mWeights[j].mVertexId;
					float weight = bone->mWeights[j].mWeight;
					m_animated_vertices[vertex_id].add_bone_data(bone_index, weight);
				}
			}
		}
	}

    const auto& white_texture = Singleton<Renderer>::get().get_white_texture();
    if (scene->HasMaterials())
    {
        const auto parent_path = std::filesystem::path{ m_filepath }.parent_path();

        m_materials.resize(scene->mNumMaterials);
        for (u32 i = 0; i < scene->mNumMaterials; ++i)
        {
            const auto* ai_material = scene->mMaterials[i];
            auto ai_material_name = ai_material->GetName();
            auto material = backend::material::create(
                m_mesh_shader,
                ai_material_name.data
            );
            auto material_asset = material_asset::create(material);

            KB_CORE_INFO("[mesh]: Adding material {} '{}'", i, ai_material_name.data);

            aiString ai_texture_path;

            glm::vec3 albedo_color{ 0.8f };
            f32 emission = 0.0f;
            aiColor3D ai_color, ai_emission;
            if (ai_material->Get(AI_MATKEY_COLOR_DIFFUSE, ai_color) == AI_SUCCESS)
            {
                auto srgb_from_linear = [](float linear_value)
                    {
                        return linear_value <= 0.0031308f
                            ? linear_value * 12.92f
                            : powf(linear_value, 1.0f / 2.2f) * 1.055f - 0.055f;
                    };
                albedo_color = {
                    srgb_from_linear(ai_color.r),
                    srgb_from_linear(ai_color.g),
                    srgb_from_linear(ai_color.b)
                };
            }

            if (ai_material->Get(AI_MATKEY_COLOR_EMISSIVE, ai_emission) == AI_SUCCESS)
            {
                emission = ai_emission.r;
            }

            material_asset->SetAlbedoColor(albedo_color);
            material_asset->SetEmission(emission);

            // TODO: roughness
            // TODO: relfectivity

            // Try load albedo
            {
                // Check for PBR albedo
                bool has_albedo_map = ai_material->GetTexture(AI_MATKEY_BASE_COLOR_TEXTURE, &ai_texture_path) == AI_SUCCESS;
                if (!has_albedo_map)
                {
                    // Check for diffuse texture
                    has_albedo_map = ai_material->GetTexture(aiTextureType_DIFFUSE, 0, &ai_texture_path) == AI_SUCCESS;
                }

                if (has_albedo_map)
                {
                    KB_CORE_INFO("[mesh]: Trying to load albedo map");
                    virtual_texture_handle texture_handle;
                    if (auto ai_texture_embedded = scene->GetEmbeddedTexture(ai_texture_path.C_Str()))
                    {
                        const u32 width = ai_texture_embedded->mWidth;
                        const u32 height = ai_texture_embedded->mHeight;
                        // Create a texture that is handled owned by the renderer
                        texture_handle = Singleton<Renderer>::get().create_texture(
                            // TODO: should just be file name
                            std::string_view{ ai_texture_path.C_Str() },
                            backend::texture_specification_t{
                                .m_format = backend::image_format_t::RGBA,
                                .m_width = width,
                                .m_height = height,
                            },
                            ai_texture_embedded->pcData
                        );
                    }
                    else
                    {
                        KB_CORE_INFO(
                            "[mesh]: Loading material albedo map from '{}'",
                            ai_texture_path.C_Str()
                        );
                        // Create a texture that is owned by the renderer
                        texture_handle = Singleton<Renderer>::get().create_texture(
                            parent_path / std::filesystem::path{ ai_texture_path.C_Str() }
                        );
                    }

                    // TODO: material asset should take a handle instead
                    material_asset->SetAlbedoMap(Singleton<Renderer>::get().get_texture_2d(texture_handle));
                    material_asset->SetAlbedoColor(glm::vec3{ 1.0f });
                }
            }

            // Try load normal
            {
                KB_CORE_INFO("[mesh]: Trying to load normal map");
                // Check for PBR albedo
                bool has_normal_map = ai_material->GetTexture(aiTextureType_NORMALS, 0, &ai_texture_path) == AI_SUCCESS;
                if (has_normal_map)
                {
                    virtual_texture_handle texture_handle;
                    if (auto ai_texture_embedded = scene->GetEmbeddedTexture(ai_texture_path.C_Str()))
                    {
                        const u32 width = ai_texture_embedded->mWidth;
                        const u32 height = ai_texture_embedded->mHeight;
                        // Create a texture that is handled owned by the renderer
                        texture_handle = Singleton<Renderer>::get().create_texture(
                            // TODO: should just be file name
                            std::string_view{ ai_texture_path.C_Str() },
                            backend::texture_specification_t{
                                .m_format = backend::image_format_t::RGBA,
                                .m_width = width,
                                .m_height = height,
                            },
                            ai_texture_embedded->pcData
                        );
                    }
                    else
                    {
                        KB_CORE_INFO(
                            "[mesh]: Loading material normal map from '{}'",
                            ai_texture_path.C_Str()
                        );
                        // Create a texture that is owned by the renderer
                        texture_handle = Singleton<Renderer>::get().create_texture(
                            parent_path / std::filesystem::path{ ai_texture_path.C_Str() }
                        );
                    }

                    // TODO: material asset should take a handle instead
                    material_asset->SetNormalMap(Singleton<Renderer>::get().get_texture_2d(texture_handle));
                    material_asset->SetUseNormalMap(true);
                }
            }

            // TODO: roughness map
            // TODO: metalness map

            m_materials[i] = material_asset;
        }
    }
    else
    {
        if (scene->HasMeshes())
        {
            m_materials.push_back(material_asset::create(backend::material::create(m_mesh_shader)));
        }
    }

    switch (renderer_pipeline)
    {
    case renderer_pipeline_type_t::pbr:
    {
        break;
    }
    case renderer_pipeline_type_t::basic:
    {
        KB_CORE_INFO("[mesh]: Setting default material values for basic pipeline");
        for (auto& material_asset : m_materials)
        {
            auto& material = material_asset->get_material();
            material->set("u_MaterialUniforms.AmbientStrength", 0.05f);
            material->set("u_MaterialUniforms.DiffuseStrength", 1.0f);
            material->set("u_MaterialUniforms.SpecularStrength", 0.3f);
        }
        break;
    }
    default:
        KB_CORE_ASSERT(false, "[mesh]: Unhandled renderer pipeline type!");
    }

	if (m_is_animated)
		m_vertex_buffer = backend::vertex_buffer::create(m_animated_vertices.data(), static_cast<u32>(m_animated_vertices.size()) * sizeof(animated_vertex_t));
	else
		m_vertex_buffer = backend::vertex_buffer::create(m_static_vertices.data(), static_cast<u32>(m_static_vertices.size()) * sizeof(vertex_t));


	m_index_buffer = backend::index_buffer::create(m_indices.data(), static_cast<u32>(m_indices.size() * sizeof(Index)));

    m_importer.reset();
    m_node_map.clear();
}

MeshData::MeshData(
    mesh_handle p_handle,
    const std::vector<vertex_t>& p_vertices,
    const std::vector<Index>& indices,
    const glm::mat4& transform
)
    : m_static_vertices{ p_vertices }, m_indices{ indices }, m_handle{ p_handle }
{
    sub_mesh_t sub_mesh;
    sub_mesh.BaseVertex = 0;
    sub_mesh.BaseIndex = 0;
    sub_mesh.IndexCount = static_cast<u32>(indices.size()) * 3u;
    sub_mesh.Transform = transform;
    sub_mesh.Material_index = 0;
    m_sub_meshes.push_back(sub_mesh);

    m_vertex_buffer = backend::vertex_buffer::create(m_static_vertices.data(), static_cast<u32>(m_static_vertices.size() * sizeof(vertex_t)));

    KB_CORE_TRACE("sizeof Index {0}", sizeof(Index));
    m_index_buffer = backend::index_buffer::create(m_indices.data(), static_cast<u32>(m_indices.size() * sizeof(Index)));

    arc<material_asset> material_asset;
    switch (get_renderer_pipeline_type())
    {
    case renderer_pipeline_type_t::pbr:
    {
        m_mesh_shader = get_shader_library()->get(shader_library::k_pbr_static_shader_name);
        auto mat = backend::material::create(m_mesh_shader, "default-pbr-material");
        // TODO: set default values
        material_asset = material_asset::create(mat);
        break;
    }
    case renderer_pipeline_type_t::basic:
    {
        m_mesh_shader = get_shader_library()->get(shader_library::k_diffuse_static_shader_name);
        auto mat = backend::material::create(m_mesh_shader, "default-basic-material");
        mat->set("u_MaterialUniforms.AmbientStrength", 0.05f);
        mat->set("u_MaterialUniforms.DiffuseStrength", 1.0f);
        mat->set("u_MaterialUniforms.SpecularStrength", 0.5f);
        mat->set("u_MaterialUniforms.AlbedoColor", glm::vec3{ 1.0f });
        material_asset = material_asset::create(mat);
        break;
    }
    }

    KB_CORE_ASSERT(material_asset, "[mesh_data]: Material asset must be set!");
	m_materials.push_back(material_asset);
}

MeshData::~MeshData()
{

}

void MeshData::set_sub_meshes(const std::vector<sub_mesh_t>& p_sub_meshes)
{
	if (!p_sub_meshes.empty())
		m_sub_meshes = p_sub_meshes;
	else
		KB_CORE_ERROR("Trying to set empty submesh array!");
}

const aiNodeAnim* MeshData::FindNodeAnim(const aiAnimation* animation, const std::string& node_name)
{
	for (u32 i = 0; i < animation->mNumChannels; ++i)
	{
		const aiNodeAnim* node_anim = animation->mChannels[i];
		if (std::string{ node_anim->mNodeName.data } == node_name)
			return node_anim;
	}

	return nullptr;
}

u32 MeshData::FindPosition(float animation_time, const aiNodeAnim* root_node_anim)
{
	for (u32 i = 0; i < root_node_anim->mNumPositionKeys - 1; ++i)
	{
		if (animation_time < static_cast<float>(root_node_anim->mPositionKeys[i + 1].mTime))
			return i;
	}

	return 0;
}

u32 MeshData::FindRotation(float animation_time, const aiNodeAnim* root_node_anim)
{
	for (u32 i = 0; i < root_node_anim->mNumRotationKeys - 1; ++i)
	{
		if (animation_time < static_cast<float>(root_node_anim->mRotationKeys[i + 1].mTime))
			return i;
	}

	return 0;
}

u32 MeshData::FindScaling(float animation_time, const aiNodeAnim* root_node_anim)
{
	for (u32 i = 0; i < root_node_anim->mNumScalingKeys - 1; ++i)
	{
		if (animation_time < static_cast<float>(root_node_anim->mScalingKeys[i + 1].mTime))
			return i;
	}

	return 0;
}

glm::vec3 MeshData::InterpolateTranslation(float animation_time, const aiNodeAnim* node_anim)
{
	KB_CORE_ASSERT(false, "not implemented");
	return glm::vec3{ 1.0f };
}

glm::quat MeshData::InterpolateRotation(float animation_time, const aiNodeAnim* node_anim)
{
	KB_CORE_ASSERT(false, "not implemented");
	return {};
}

glm::vec3 MeshData::InterpolateScale(float animation_time, const aiNodeAnim* node_anim)
{
	KB_CORE_ASSERT(false, "not implemented");
	return glm::vec3{ 1.0f };
}

void MeshData::ReadNodeHierarchy(
    const aiScene* p_scene,
    float animation_time,
    const aiNode* root,
    const glm::mat4& parent_transform
)
{
    const std::string name = root->mName.data;
	const aiAnimation* animation = p_scene->mAnimations[0];
	auto node_transform = util::Mat4FromAssimpMat4(root->mTransformation);
	const aiNodeAnim* node_anim = FindNodeAnim(animation, name);

	if (node_anim)
	{
        const glm::vec3 translation = InterpolateTranslation(animation_time, node_anim);
        const glm::mat4 translation_mat = glm::translate(glm::mat4{ 1.0f }, translation);

        const glm::quat rot = InterpolateRotation(animation_time, node_anim);
        const glm::mat4 rot_mat = glm::toMat4(rot);

        const glm::vec3 scale = InterpolateScale(animation_time, node_anim);
        const glm::mat4 scale_mat = glm::scale(glm::mat4{ 1.0f }, scale);

		node_transform = translation_mat * rot_mat * scale_mat;
	}

    const glm::mat4 transform = parent_transform * node_transform;

	if (m_bone_mapping.find(name) != m_bone_mapping.end())
	{
        const u32 bone_index = m_bone_mapping[name];
		m_bone_info[bone_index].Final_transformation = m_inverse_transform * transform * m_bone_info[bone_index].Bone_offset;
	}

	for (u32 i = 0; i < root->mNumChildren; ++i)
		ReadNodeHierarchy(p_scene, animation_time, root->mChildren[i], transform);
}

void MeshData::TraverseNodes(aiNode* root, const glm::mat4& parent_transform, u32 level)
{
    const auto local_transform = util::Mat4FromAssimpMat4(root->mTransformation);
    const auto transform = parent_transform * local_transform;
	m_node_map[root].resize(root->mNumMeshes);
	for (u32 i = 0; i < root->mNumMeshes; ++i)
	{
        const u32 mesh_index = root->mMeshes[i];
		auto& sub_mesh = m_sub_meshes[mesh_index];
		sub_mesh.node_name = root->mName.C_Str();
		sub_mesh.Transform = transform;
		sub_mesh.Local_transform = local_transform;
		m_node_map[root][i] = mesh_index;
	}

	for (u32 i = 0; i < root->mNumChildren; ++i)
		TraverseNodes(root->mChildren[i], transform, level + 1);
}

Mesh::Mesh(arc<MeshData> mesh_data)
	: m_mesh_data{ std::move(mesh_data) }
{
	set_sub_meshes({});
    util::init_material_table(m_material_table, m_mesh_data->get_materials());
}

Mesh::Mesh(const arc<Mesh>& other)
	: m_mesh_data{ other->m_mesh_data }
{
	set_sub_meshes({});
    util::init_material_table(m_material_table, m_mesh_data->get_materials());
}

Mesh::Mesh(arc<MeshData> mesh_data, const std::vector<u32>& sub_meshes)
    : m_mesh_data{ std::move(mesh_data) }
{
	set_sub_meshes(sub_meshes);
    util::init_material_table(m_material_table, m_mesh_data->get_materials());
}

void Mesh::OnUpdate(Timestep ts)
{
	KB_CORE_WARN("Mesh OnUpdate() not implemented!");
}

void Mesh::set_sub_meshes(const std::vector<u32>& p_sub_meshes)
{
	if (!p_sub_meshes.empty())
		m_submeshes = p_sub_meshes;
	else
	{
		const auto& sub_meshes = m_mesh_data->get_sub_meshes();
		m_submeshes.resize(sub_meshes.size());
		for (u32 i = 0; i < sub_meshes.size(); ++i)
			m_submeshes[i] = i;
	}
}

arc<Mesh> MeshFactory::CreateCube(float side_length, Entity entity)
{
	std::vector<vertex_t> verts;
	verts.resize(8);
	verts[0].Position = vec3_packed{ -side_length / 2.0f, -side_length / 2.0f,  side_length / 2.0f };
	verts[1].Position = vec3_packed{ side_length / 2.0f, -side_length / 2.0f,  side_length / 2.0f };
	verts[2].Position = vec3_packed{ side_length / 2.0f,  side_length / 2.0f,  side_length / 2.0f };
	verts[3].Position = vec3_packed{ -side_length / 2.0f,  side_length / 2.0f,  side_length / 2.0f };
	verts[4].Position = vec3_packed{ -side_length / 2.0f, -side_length / 2.0f, -side_length / 2.0f };
	verts[5].Position = vec3_packed{ side_length / 2.0f, -side_length / 2.0f, -side_length / 2.0f };
	verts[6].Position = vec3_packed{ side_length / 2.0f,  side_length / 2.0f, -side_length / 2.0f };
	verts[7].Position = vec3_packed{ -side_length / 2.0f,  side_length / 2.0f, -side_length / 2.0f };

	verts[0].Normal = vec3_packed{ -1.0f, -1.0f,  1.0f };
	verts[1].Normal = vec3_packed{ 1.0f, -1.0f,  1.0f };
	verts[2].Normal = vec3_packed{ 1.0f,  1.0f,  1.0f };
	verts[3].Normal = vec3_packed{ -1.0f,  1.0f,  1.0f };
	verts[4].Normal = vec3_packed{ -1.0f, -1.0f, -1.0f };
	verts[5].Normal = vec3_packed{ 1.0f, -1.0f, -1.0f };
	verts[6].Normal = vec3_packed{ 1.0f,  1.0f, -1.0f };
	verts[7].Normal = vec3_packed{ -1.0f,  1.0f, -1.0f };


	// #FIXME this is garbage data
	verts[0].Binormal = vec3_packed{ 1.0f, 1.0f, 1.0f };
	verts[1].Binormal = vec3_packed{ 1.0f, 1.0f, 1.0f };
	verts[2].Binormal = vec3_packed{ 1.0f, 1.0f, 1.0f };
	verts[3].Binormal = vec3_packed{ 1.0f, 1.0f, 1.0f };
	verts[4].Binormal = vec3_packed{ 1.0f, 1.0f, 1.0f };
	verts[5].Binormal = vec3_packed{ 1.0f, 1.0f, 1.0f };
	verts[6].Binormal = vec3_packed{ 1.0f, 1.0f, 1.0f };
	verts[7].Binormal = vec3_packed{ 1.0f, 1.0f, 1.0f };

	// #FIXME this is garbage data
	verts[0].Tangent = vec3_packed{ 1.0f, 1.0f, 1.0f };
	verts[1].Tangent = vec3_packed{ 1.0f, 1.0f, 1.0f };
	verts[2].Tangent = vec3_packed{ 1.0f, 1.0f, 1.0f };
	verts[3].Tangent = vec3_packed{ 1.0f, 1.0f, 1.0f };
	verts[4].Tangent = vec3_packed{ 1.0f, 1.0f, 1.0f };
	verts[5].Tangent = vec3_packed{ 1.0f, 1.0f, 1.0f };
	verts[6].Tangent = vec3_packed{ 1.0f, 1.0f, 1.0f };
	verts[7].Tangent = vec3_packed{ 1.0f, 1.0f, 1.0f };

	// #FIXME this is garbage data
	verts[0].TexCoord = vec2_packed{ 1.0f, 1.0f, };
	verts[1].TexCoord = vec2_packed{ 1.0f, 1.0f, };
	verts[2].TexCoord = vec2_packed{ 1.0f, 1.0f, };
	verts[3].TexCoord = vec2_packed{ 1.0f, 1.0f, };
	verts[4].TexCoord = vec2_packed{ 1.0f, 1.0f, };
	verts[5].TexCoord = vec2_packed{ 1.0f, 1.0f, };
	verts[6].TexCoord = vec2_packed{ 1.0f, 1.0f, };
	verts[7].TexCoord = vec2_packed{ 1.0f, 1.0f, };

	std::vector<Index> indices;
	indices.resize(12);
	//Front
	indices[0]  = { 0, 1, 2 };
	indices[1]  = { 2, 3, 0 };

	//Right
	indices[2] = { 1, 5, 6 };
	indices[3] = { 6, 2, 1 };

	//Back
	indices[4] = { 7, 6, 5 };
	indices[5] = { 5, 4, 7 };

	// Left
	indices[6] = { 4, 0, 3 };
	indices[7] = { 3, 7, 4 };

	//Bottom
	indices[8] = { 4, 5, 1 };
	indices[9] = { 1, 0, 4 };

	//Top
	indices[10] = { 3, 2, 6 };
	indices[11] = { 6, 7, 3 };

	return arc<Mesh>::Create(arc<MeshData>::Create(
        k_cube_mesh_handle,
        verts,
        indices,
        glm::mat4{ 1.0f }
    ));
}
} // end namespace kb::render
