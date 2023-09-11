#include "kablunkpch.h"

#include "Kablunk/Core/Core.h"
#include "Kablunk/Renderer/Mesh.h"
#include "Kablunk/Renderer/Shader.h"
#include "Kablunk/Renderer/RenderCommand.h"
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

namespace kb
{
	static constexpr const uint32_t s_mesh_import_flags =
		aiProcess_Triangulate | aiProcess_GenNormals | aiProcess_CalcTangentSpace | aiProcess_GenUVCoords | aiProcess_ValidateDataStructure;
	
	namespace Utils {

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

	}

	MeshData::MeshData(const std::string& filepath, Entity entity)
		: m_filepath{ filepath }
	{
		KB_CORE_TRACE("Loading mesh: '{0}'", filepath.c_str());

		m_importer = CreateScope<Assimp::Importer>();
		
		const aiScene* scene = m_importer->ReadFile(m_filepath, s_mesh_import_flags);
		if (!scene || !scene->HasMeshes())
		{
			KB_CORE_ERROR("Failed to load mesh file: '{0}'", m_filepath);
			return;
		}

		m_scene = scene;

		m_is_animated = m_scene->mAnimations != nullptr;
		if (m_is_animated)
			KB_CORE_INFO("ANIMATED MESH!");

		if (render::get_render_pipeline() == RendererPipelineDescriptor::PHONG_DIFFUSE)
		{
			m_mesh_shader = m_is_animated ? render::get_shader("Kablunk_diffuse_anim") : render::get_shader("Kablunk_diffuse_static");
		}
		else if (render::get_render_pipeline() == RendererPipelineDescriptor::PBR)
		{
			KB_CORE_ASSERT(false, "not implemented!");
		}

		m_inverse_transform = glm::inverse(Utils::Mat4FromAssimpMat4(scene->mRootNode->mTransformation));

		size_t vertex_count = 0;
		size_t index_count = 0;

		m_sub_meshes.reserve(scene->mNumMeshes);
		for (size_t m = 0; m < scene->mNumMeshes; ++m)
		{
			aiMesh* mesh = scene->mMeshes[m];

			Submesh& submesh = m_sub_meshes.emplace_back();
			submesh.BaseVertex = static_cast<uint32_t>(vertex_count);
			submesh.BaseIndex = static_cast<uint32_t>(index_count);
			submesh.Material_index = mesh->mMaterialIndex;
			submesh.VertexCount = mesh->mNumVertices;
			submesh.IndexCount = mesh->mNumFaces * 3;
			submesh.mesh_name = mesh->mName.C_Str();

			vertex_count += mesh->mNumVertices;
			index_count += submesh.IndexCount;

			if (m_is_animated)
			{
				for (size_t i = 0; i < mesh->mNumVertices; ++i)
				{
					AnimatedVertex v;
					v.Position = { mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z };
					v.Normal = { mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z };
					v.EntityID = static_cast<int32_t>(entity);

					if (mesh->HasTangentsAndBitangents())
					{
						v.Tangent = { mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z };
						v.Binormal = { mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z };
					}

					if (mesh->HasTextureCoords(0))
					{
						// #FIXME figure out how to deal with multiple textures
						v.TexCoord = { mesh->mTextureCoords[0]->x, mesh->mTextureCoords[0]->y };
					}

					m_animated_vertices.push_back(v);
				}
			}
			else
			{
				for (size_t i = 0; i < mesh->mNumVertices; ++i)
				{
					Vertex v;
					v.Position = { mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z };
					v.Normal = { mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z };

					if (mesh->HasTangentsAndBitangents())
					{
						v.Tangent = { mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z };
						v.Binormal = { mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z };
					}

					if (mesh->HasTextureCoords(0))
					{
						// #FIXME figure out how to deal with multiple textures
						v.TexCoord = { mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y };
					}

					m_static_vertices.push_back(v);
				}
			}

			
			for (uint32_t i = 0; i < mesh->mNumFaces; ++i)
			{
				KB_CORE_ASSERT(mesh->mFaces[i].mNumIndices == 3, "Must have 3 indices");
				Index index = { mesh->mFaces[i].mIndices[0], mesh->mFaces[i].mIndices[1], mesh->mFaces[i].mIndices[2] };
				m_indices.push_back(index);

				if (!m_is_animated)
				{
					m_triangle_cache[m].emplace_back(
						m_static_vertices[index.V1 + submesh.BaseVertex], 
						m_static_vertices[index.V2 + submesh.BaseVertex], 
						m_static_vertices[index.V3 + submesh.BaseVertex]
					);
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
				Submesh& submesh = m_sub_meshes[i];

				for (size_t b = 0; b < mesh->mNumBones; ++b)
				{
					aiBone* bone = mesh->mBones[b];
					std::string bone_name = bone->mName.data;
					uint32_t bone_index = 0;

					if (m_bone_mapping.find(bone_name) == m_bone_mapping.end())
					{
						bone_index = m_bone_count++;
						BoneInfo& bone_info = m_bone_info.emplace_back();
						bone_info.Bone_offset = Utils::Mat4FromAssimpMat4(bone->mOffsetMatrix);
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
						m_animated_vertices[vertex_id].AddBoneData(bone_index, weight);
					}
				}
			}
			
		}

		ref<Texture2D> white_texture = Application::Get().get_renderer_2d()->get_white_texture();
		if (scene->HasMaterials() && render::get_render_pipeline() == RendererPipelineDescriptor::PBR)
		{
			m_textures.resize(scene->mNumMaterials);
			m_materials.resize(scene->mNumMaterials);

			for (uint32_t i = 0; i < scene->mNumMaterials; ++i)
			{
				auto ai_material = scene->mMaterials[i];
				auto ai_material_name = ai_material->GetName();

				auto mat = Material::Create(m_mesh_shader, ai_material_name.data);
				m_materials[i] = mat;

				aiString ai_texture_path;
				uint32_t texture_count = ai_material->GetTextureCount(aiTextureType_DIFFUSE);

				glm::vec3 albedo_color{ 0.8f };
				float emission = 0.0f;
				aiColor3D ai_color, ai_emission;

				float shininess, metalness;

				if (ai_material->Get(AI_MATKEY_COLOR_DIFFUSE, ai_color) == AI_SUCCESS)
					albedo_color = { ai_color.r, ai_color.g, ai_color.b };

				if (ai_material->Get(AI_MATKEY_COLOR_EMISSIVE, ai_emission) == AI_SUCCESS)
					emission = ai_emission.r;

				if (ai_material->Get(AI_MATKEY_SHININESS, shininess) != AI_SUCCESS)
					shininess = 80.0f;

				if (ai_material->Get(AI_MATKEY_REFLECTIVITY, metalness) != AI_SUCCESS)
					metalness = 0.0f;

				float roughness = 1.0f - glm::sqrt(shininess / 100.0f);

				mat->Set("u_MaterialUniforms.AlbedoColor", albedo_color);
				mat->Set("u_MaterialUniforms.Emission", emission);

				// Albedo
				bool has_albedo_map = ai_material->GetTexture(aiTextureType_DIFFUSE, 0, &ai_texture_path) == AI_SUCCESS;
				bool fallback = !has_albedo_map;
				if (has_albedo_map)
				{
					std::filesystem::path path = m_filepath;
					auto parent_path = path.parent_path();
					parent_path /= std::string(ai_texture_path.data);
					std::string texture_path = parent_path.string();

					// #TODO texture properties
					//TextureProperties props;
					//props.SRGB = true;
					auto texture = Texture2D::Create(texture_path);
					if (texture)
					{
						m_textures[i] = texture;
						mat->Set("u_AlbedoTexture", texture);
						mat->Set("u_MaterialUniforms.AlbedoColor", glm::vec3{ 1.0f });
					}
					else
					{
						m_textures[i] = white_texture;
						fallback = true;
					}
				}
				
				if (fallback)
					mat->Set("u_AlbedoTexture", white_texture);

				// Normal Map
				bool has_normal_map = ai_material->GetTexture(aiTextureType_NORMALS, 0, &ai_texture_path) == AI_SUCCESS;
				fallback = !has_normal_map;
				if (has_normal_map)
				{
					std::filesystem::path path = m_filepath;
					auto parent_path = path.parent_path();
					parent_path /= std::string(ai_texture_path.data);
					std::string texture_path = parent_path.string();

					// #TODO texture properties
					//TextureProperties props;
					//props.SRGB = true;
					auto texture = Texture2D::Create(texture_path);
					if (texture)
					{
						m_textures[i] = texture;
						mat->Set("u_NormalTexture", texture);
						mat->Set("u_MaterialUniforms.UseNormalMap", true);
					}
					else
					{
						m_textures[i] = white_texture;
						fallback = true;
					}
				}

				if (fallback)
				{
					mat->Set("u_NormalTexture", white_texture);
					mat->Set("u_MaterialUniforms.UseNormalMap", false);
				}

				// Roughness map
				bool has_roughness_map = ai_material->GetTexture(aiTextureType_SHININESS, 0, &ai_texture_path) == AI_SUCCESS;
				fallback = !has_roughness_map;
				if (has_roughness_map)
				{
					// TODO: Temp - this should be handled by Hazel's filesystem
					std::filesystem::path path = m_filepath;
					auto parent_path = path.parent_path();
					parent_path /= std::string(ai_texture_path.data);
					std::string texture_path = parent_path.string();
					auto texture = Texture2D::Create(texture_path);
					if (texture)
					{
						m_textures.push_back(texture);
						mat->Set("u_RoughnessTexture", texture);
						mat->Set("u_MaterialUniforms.Roughness", 1.0f);
					}
					else
						fallback = true;
					
				}

				if (fallback)
				{
					mat->Set("u_RoughnessTexture", white_texture);
					mat->Set("u_MaterialUniforms.Roughness", roughness);
				}

				// Metalness Textures
				bool metalness_texture_found = false;
				for (uint32_t p = 0; p < ai_material->mNumProperties; p++)
				{
					auto prop = ai_material->mProperties[p];

					if (prop->mType == aiPTI_String)
					{
						uint32_t strLength = *(uint32_t*)prop->mData;
						std::string str(prop->mData + 4, strLength);

						std::string key = prop->mKey.data;
						if (key == "$raw.ReflectionFactor|file")
						{
							// TODO: Temp - this should be handled by Hazel's filesystem
							std::filesystem::path path = m_filepath;
							auto parent_path = path.parent_path();
							parent_path /= str;
							std::string texture_path = parent_path.string();
							auto texture = Texture2D::Create(texture_path);
							if (texture)
							{
								metalness_texture_found = true;
								m_textures.push_back(texture);
								mat->Set("u_MetalnessTexture", texture);
								mat->Set("u_MaterialUniforms.Metalness", 1.0f);
							}

							break;
						}
					}
				}

				fallback = !metalness_texture_found;
				if (fallback)
				{
					mat->Set("u_MetalnessTexture", white_texture);
					mat->Set("u_MaterialUniforms.Metalness", metalness);
				}

			}
		}
		else
		{
			if (render::get_render_pipeline() == RendererPipelineDescriptor::PBR)
			{
				auto mat = Material::Create(m_mesh_shader, "Kablunk-Default");
				// Props
				mat->Set("u_MaterialUniforms.AlbedoColor", glm::vec3(0.8f));
				mat->Set("u_MaterialUniforms.Emission", 0.0f);
				mat->Set("u_MaterialUniforms.Metalness", 0.0f);
				mat->Set("u_MaterialUniforms.Roughness", 0.8f);
				mat->Set("u_MaterialUniforms.UseNormalMap", false);

				// textures
				mat->Set("u_AlbedoTexture", white_texture);
				mat->Set("u_MetalnessTexture", white_texture);
				mat->Set("u_RoughnessTexture", white_texture);
				m_materials.push_back(mat);
			}
			else if (render::get_render_pipeline() == RendererPipelineDescriptor::PHONG_DIFFUSE)
			{
				auto mat = Material::Create(m_mesh_shader, "Kablunk-PhongDefault");
				mat->Set("u_MaterialUniforms.AmbientStrength", 0.3f);
				mat->Set("u_MaterialUniforms.DiffuseStrength", 1.0f);
				mat->Set("u_MaterialUniforms.SpecularStrength", 0.5f);
				m_materials.push_back(mat);
			}
		}


		if (m_is_animated)
			m_vertex_buffer = VertexBuffer::Create(m_animated_vertices.data(), (uint32_t)m_animated_vertices.size() * sizeof(AnimatedVertex));
		else
			m_vertex_buffer = VertexBuffer::Create(m_static_vertices.data(), (uint32_t)m_static_vertices.size() * sizeof(Vertex));
		

		m_index_buffer = IndexBuffer::Create(m_indices.data(), (uint32_t)(m_indices.size() * sizeof(Index)));
	}

	MeshData::MeshData(const std::vector<Vertex>& verticies, const std::vector<Index>& indices, const glm::mat4& transform)
		: m_static_vertices{ verticies }, m_indices{ indices }
	{
		Submesh submesh;
		submesh.BaseVertex = 0;
		submesh.BaseIndex = 0;
		submesh.IndexCount = (uint32_t)indices.size() * 3u;
		submesh.Transform = transform;
		submesh.Material_index = 0;
		m_sub_meshes.push_back(submesh);

		m_vertex_buffer = VertexBuffer::Create(m_static_vertices.data(), (uint32_t)(m_static_vertices.size() * sizeof(Vertex)));

		KB_CORE_TRACE("sizeof Index {0}", sizeof(Index));
		m_index_buffer = IndexBuffer::Create(m_indices.data(), (uint32_t)(m_indices.size() * sizeof(Index)));

		if (render::get_render_pipeline() == RendererPipelineDescriptor::PHONG_DIFFUSE)
		{
			m_mesh_shader = render::get_shader_library()->Get("Kablunk_diffuse_static");
			auto mat = Material::Create(m_mesh_shader, "Kablunk-PhongDefault");
			mat->Set("u_MaterialUniforms.AmbientStrength", 0.3f);
			mat->Set("u_MaterialUniforms.DiffuseStrength", 1.0f);
			mat->Set("u_MaterialUniforms.SpecularStrength", 0.5f);
			m_materials.push_back(mat);
		}
		else if (render::get_render_pipeline() == RendererPipelineDescriptor::PBR)
		{
			KB_CORE_ASSERT(false, "not implemented!");
		}
	}

	MeshData::~MeshData()
	{

	}

	void MeshData::SetSubmeshes(const std::vector<Submesh>& submeshes)
	{
		if (!submeshes.empty())
			m_sub_meshes = submeshes;
		else
			KB_CORE_ERROR("Trying to set empty submesh array!");
	}

	const aiNodeAnim* MeshData::FindNodeAnim(const aiAnimation* animation, const std::string& node_name)
	{
		for (uint32_t i = 0; i < animation->mNumChannels; ++i)
		{
			const aiNodeAnim* node_anim = animation->mChannels[i];
			if (std::string{ node_anim->mNodeName.data } == node_name)
				return node_anim;
		}

		return nullptr;
	}

	uint32_t MeshData::FindPosition(float animation_time, const aiNodeAnim* root_node_anim)
	{
		for (uint32_t i = 0; i < root_node_anim->mNumPositionKeys - 1; ++i)
		{
			if (animation_time < static_cast<float>(root_node_anim->mPositionKeys[i + 1].mTime))
				return i;
		}

		return 0;
	}

	uint32_t MeshData::FindRotation(float animation_time, const aiNodeAnim* root_node_anim)
	{
		for (uint32_t i = 0; i < root_node_anim->mNumRotationKeys - 1; ++i)
		{
			if (animation_time < static_cast<float>(root_node_anim->mRotationKeys[i + 1].mTime))
				return i;
		}

		return 0;
	}

	uint32_t MeshData::FindScaling(float animation_time, const aiNodeAnim* root_node_anim)
	{
		for (uint32_t i = 0; i < root_node_anim->mNumScalingKeys - 1; ++i)
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

	void MeshData::ReadNodeHierarchy(float animation_time, const aiNode* root, const glm::mat4& parent_transform)
	{
		std::string name = root->mName.data;
		const aiAnimation* animation = m_scene->mAnimations[0];
		auto node_transform = Utils::Mat4FromAssimpMat4(root->mTransformation);
		const aiNodeAnim* node_anim = FindNodeAnim(animation, name);

		if (node_anim)
		{
			glm::vec3 translation = InterpolateTranslation(animation_time, node_anim);
			glm::mat4 translation_mat = glm::translate(glm::mat4{ 1.0f }, translation);

			glm::quat rot = InterpolateRotation(animation_time, node_anim);
			glm::mat4 rot_mat = glm::toMat4(rot);

			glm::vec3 scale = InterpolateScale(animation_time, node_anim);
			glm::mat4 scale_mat = glm::scale(glm::mat4{ 1.0f }, scale);

			node_transform = translation_mat * rot_mat * scale_mat;
		}

		glm::mat4 transform = parent_transform * node_transform;

		if (m_bone_mapping.find(name) != m_bone_mapping.end())
		{
			uint32_t bone_index = m_bone_mapping[name];
			m_bone_info[bone_index].Final_transformation = m_inverse_transform * transform * m_bone_info[bone_index].Bone_offset;
		}

		for (uint32_t i = 0; i < root->mNumChildren; ++i)
			ReadNodeHierarchy(animation_time, root->mChildren[i], transform);
	}

	void MeshData::TraverseNodes(aiNode* root, const glm::mat4& parent_transform, uint32_t level)
	{
		auto local_transform = Utils::Mat4FromAssimpMat4(root->mTransformation);
		auto transform = parent_transform * local_transform;
		m_node_map[root].resize(root->mNumMeshes);
		for (uint32_t i = 0; i < root->mNumMeshes; ++i)
		{
			uint32_t mesh_index = root->mMeshes[i];
			auto& submesh = m_sub_meshes[mesh_index];
			submesh.node_name = root->mName.C_Str();
			submesh.Transform = transform;
			submesh.Local_transform = local_transform;
			m_node_map[root][i] = mesh_index;
		}

		for (uint32_t i = 0; i < root->mNumChildren; ++i)
			TraverseNodes(root->mChildren[i], transform, level + 1);
	}

	Mesh::Mesh(ref<MeshData> mesh_data)
		: m_mesh_data{ mesh_data }
	{
		SetSubmeshes({});

		const auto& mesh_materials = m_mesh_data->GetMaterials();
		m_material_table = ref<MaterialTable>::Create(mesh_materials.size());
		for (size_t i = 0; i < mesh_materials.size(); ++i)
			m_material_table->SetMaterial(static_cast<uint32_t>(i), ref<MaterialAsset>::Create(mesh_materials[i]));
	}

	Mesh::Mesh(const ref<Mesh>& other)
		: m_mesh_data{ other->m_mesh_data }
	{
		SetSubmeshes({});

		const auto& mesh_materials = m_mesh_data->GetMaterials();
		m_material_table = ref<MaterialTable>::Create(mesh_materials.size());
		for (size_t i = 0; i < mesh_materials.size(); ++i)
			m_material_table->SetMaterial(static_cast<uint32_t>(i), ref<MaterialAsset>::Create(mesh_materials[i]));
	}

	Mesh::Mesh(ref<MeshData> mesh_data, const std::vector<uint32_t>& submeshes)
	{
		SetSubmeshes(submeshes);

		const auto& mesh_materials = m_mesh_data->GetMaterials();
		m_material_table = ref<MaterialTable>::Create(mesh_materials.size());
		for (size_t i = 0; i < mesh_materials.size(); ++i)
			m_material_table->SetMaterial(static_cast<uint32_t>(i), ref<MaterialAsset>::Create(mesh_materials[i]));
	}

	Mesh::~Mesh()
	{

	}

	void Mesh::OnUpdate(Timestep ts)
	{
		KB_CORE_WARN("Mesh OnUpdate() not implemented!");
	}

	void Mesh::SetSubmeshes(const std::vector<uint32_t>& submeshes)
	{
		if (!submeshes.empty())
			m_submeshes = submeshes;
		else
		{
			const auto& submeshes = m_mesh_data->GetSubmeshes();
			m_submeshes.resize(submeshes.size());
			for (uint32_t i = 0; i < submeshes.size(); ++i)
				m_submeshes[i] = i;
		}
	}

	ref<Mesh> MeshFactory::CreateCube(float side_length, Entity entity)
	{
		std::vector<Vertex> verts;
		verts.resize(8);
		verts[0].Position = { -side_length / 2.0f, -side_length / 2.0f,  side_length / 2.0f };
		verts[1].Position = { side_length / 2.0f, -side_length / 2.0f,  side_length / 2.0f };
		verts[2].Position = { side_length / 2.0f,  side_length / 2.0f,  side_length / 2.0f };
		verts[3].Position = { -side_length / 2.0f,  side_length / 2.0f,  side_length / 2.0f };
		verts[4].Position = { -side_length / 2.0f, -side_length / 2.0f, -side_length / 2.0f };
		verts[5].Position = { side_length / 2.0f, -side_length / 2.0f, -side_length / 2.0f };
		verts[6].Position = { side_length / 2.0f,  side_length / 2.0f, -side_length / 2.0f };
		verts[7].Position = { -side_length / 2.0f,  side_length / 2.0f, -side_length / 2.0f };
		
		verts[0].Normal = { -1.0f, -1.0f,  1.0f };
		verts[1].Normal = { 1.0f, -1.0f,  1.0f };
		verts[2].Normal = { 1.0f,  1.0f,  1.0f };
		verts[3].Normal = { -1.0f,  1.0f,  1.0f };
		verts[4].Normal = { -1.0f, -1.0f, -1.0f };
		verts[5].Normal = { 1.0f, -1.0f, -1.0f };
		verts[6].Normal = { 1.0f,  1.0f, -1.0f };
		verts[7].Normal = { -1.0f,  1.0f, -1.0f };


		// #FIXME this is garbage data
		verts[0].Binormal = { 1.0f, 1.0f, 1.0f };
		verts[1].Binormal = { 1.0f, 1.0f, 1.0f };
		verts[2].Binormal = { 1.0f, 1.0f, 1.0f };
		verts[3].Binormal = { 1.0f, 1.0f, 1.0f };
		verts[4].Binormal = { 1.0f, 1.0f, 1.0f };
		verts[5].Binormal = { 1.0f, 1.0f, 1.0f };
		verts[6].Binormal = { 1.0f, 1.0f, 1.0f };
		verts[7].Binormal = { 1.0f, 1.0f, 1.0f };

		// #FIXME this is garbage data
		verts[0].Tangent = { 1.0f, 1.0f, 1.0f };
		verts[1].Tangent = { 1.0f, 1.0f, 1.0f };
		verts[2].Tangent = { 1.0f, 1.0f, 1.0f };
		verts[3].Tangent = { 1.0f, 1.0f, 1.0f };
		verts[4].Tangent = { 1.0f, 1.0f, 1.0f };
		verts[5].Tangent = { 1.0f, 1.0f, 1.0f };
		verts[6].Tangent = { 1.0f, 1.0f, 1.0f };
		verts[7].Tangent = { 1.0f, 1.0f, 1.0f };

		// #FIXME this is garbage data
		verts[0].TexCoord = { 1.0f, 1.0f, };
		verts[1].TexCoord = { 1.0f, 1.0f, };
		verts[2].TexCoord = { 1.0f, 1.0f, };
		verts[3].TexCoord = { 1.0f, 1.0f, };
		verts[4].TexCoord = { 1.0f, 1.0f, };
		verts[5].TexCoord = { 1.0f, 1.0f, };
		verts[6].TexCoord = { 1.0f, 1.0f, };
		verts[7].TexCoord = { 1.0f, 1.0f, };

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

		return ref<Mesh>::Create(ref<MeshData>::Create(verts, indices, glm::mat4{ 1.0f }));
	}
}
