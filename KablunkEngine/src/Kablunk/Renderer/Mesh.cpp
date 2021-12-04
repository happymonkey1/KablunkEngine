#include "kablunkpch.h"

#include "Kablunk/Core/Core.h"
#include "Kablunk/Renderer/Mesh.h"
#include "Kablunk/Renderer/Shader.h"
#include "Kablunk/Renderer/Renderer.h"
#include "Kablunk/Scene/Entity.h"

#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>

#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>


namespace Kablunk
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
		m_mesh_shader = m_is_animated ? Renderer::GetShaderLibrary()->Get("Kablunk_diffuse_anim")  : Renderer::GetShaderLibrary()->Get("Kablunk_diffuse_static");
		m_inverse_transform = glm::inverse(Utils::Mat4FromAssimpMat4(scene->mRootNode->mTransformation));

		size_t vertex_count = 0;
		size_t index_count = 0;

		m_sub_meshes.reserve(scene->mNumMeshes);
		for (size_t m = 0; m < scene->mNumMeshes; ++m)
		{
			aiMesh* mesh = scene->mMeshes[m];

			Submesh& submesh = m_sub_meshes.emplace_back();
			submesh.BaseVertex = vertex_count;
			submesh.BaseIndex = index_count;
			submesh.MaterialIndex = mesh->mMaterialIndex;
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

					m_static_vertices.push_back(v);
				}
			}

			
			for (uint32_t i = 0; i < mesh->mNumFaces; ++i)
			{
				KB_CORE_ASSERT(mesh->mFaces[i].mNumIndices == 3, "Must have 3 indices");
				Index index = { mesh->mFaces[i].mIndices[0], mesh->mFaces[i].mIndices[1], mesh->mFaces[i].mIndices[2] };
				m_indices.push_back(index);

				if (!m_is_animated)
					m_triangle_cache[i].emplace_back(m_static_vertices[index.V1], m_static_vertices[index.V2], m_static_vertices[index.V3]);
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

		// #TODO materials


		if (m_is_animated)
		{
			m_vertex_buffer = VertexBuffer::Create(m_animated_vertices.data(), (uint32_t)m_animated_vertices.size() * sizeof(AnimatedVertex));
			m_vertex_buffer_layout = {
				{ ShaderDataType::Float3, "a_Position" },
				{ ShaderDataType::Float3, "a_Normal" },
				{ ShaderDataType::Float3, "a_Tangent" },
				{ ShaderDataType::Float3, "a_Binormal" },
				{ ShaderDataType::Float2, "a_TexCoord" },
				{ ShaderDataType::Int4, "a_BoneIDs" },
				{ ShaderDataType::Float4, "a_Weights" },
				{ ShaderDataType::Int, "a_EntityID" }
			};
			m_vertex_buffer->SetLayout(m_vertex_buffer_layout);
		}
		else
		{
			m_vertex_buffer = VertexBuffer::Create(m_static_vertices.data(), (uint32_t)m_static_vertices.size() * sizeof(Vertex));
			m_vertex_buffer_layout = {
				{ ShaderDataType::Float3, "a_Position" },
				{ ShaderDataType::Float3, "a_Normal" },
				{ ShaderDataType::Float3, "a_Tangent" },
				{ ShaderDataType::Float3, "a_Binormal" },
				{ ShaderDataType::Float2, "a_TexCoord" },
				{ ShaderDataType::Int, "a_EntityID" }
			};
			m_vertex_buffer->SetLayout(m_vertex_buffer_layout);
		}

		m_index_buffer = IndexBuffer::Create(m_indices.data(), (uint32_t)(m_indices.size() * sizeof(Index) / sizeof(uint32_t)));
	}

	MeshData::MeshData(const std::vector<Vertex>& verticies, const std::vector<Index>& indices, const glm::mat4& transform)
		: m_static_vertices{ verticies }, m_indices{ indices }
	{
		Submesh submesh;
		submesh.BaseVertex = 0;
		submesh.BaseIndex = 0;
		submesh.IndexCount = (uint32_t)indices.size() * 3u;
		submesh.Transform = transform;
		m_sub_meshes.push_back(submesh);

		
		m_mesh_shader = Renderer::GetShaderLibrary()->Get("Kablunk_diffuse_static");

		m_vertex_buffer = VertexBuffer::Create(m_static_vertices.data(), (uint32_t)(m_static_vertices.size() * sizeof(Vertex)));

		m_vertex_buffer_layout = {
			{ ShaderDataType::Float3, "a_Position" },
			{ ShaderDataType::Float3, "a_Normal" },
			{ ShaderDataType::Float3, "a_Tangent" },
			{ ShaderDataType::Float3, "a_Binormal" },
			{ ShaderDataType::Float2, "a_TexCoord" },
			{ ShaderDataType::Int, "a_EntityID" }
		};
		m_vertex_buffer->SetLayout(m_vertex_buffer_layout);

		KB_CORE_TRACE("sizeof Index {0}", sizeof(Index));
		m_index_buffer = IndexBuffer::Create(m_indices.data(), (uint32_t)(m_indices.size() * sizeof(Index) / sizeof(uint32_t)));
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

	Mesh::Mesh(Ref<MeshData> mesh_data)
		: m_mesh_data{ mesh_data }
	{
		m_vertex_array = VertexArray::Create();
		m_vertex_array->AddVertexBuffer(m_mesh_data->GetVertexBuffer());
		m_vertex_array->SetIndexBuffer(m_mesh_data->GetIndexBuffer());
	}

	Mesh::Mesh(const Ref<Mesh>& other)
		: m_mesh_data{ other->m_mesh_data }
	{
		m_vertex_array = VertexArray::Create();
		m_vertex_array->AddVertexBuffer(m_mesh_data->GetVertexBuffer());
		m_vertex_array->SetIndexBuffer(m_mesh_data->GetIndexBuffer());
	}

	Mesh::~Mesh()
	{

	}

	void Mesh::OnUpdate(Timestep ts)
	{
		KB_CORE_WARN("Mesh OnUpdate() not implemented!");
	}

	Ref<Mesh> MeshFactory::CreateCube(float side_length, Entity entity)
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


		verts[0].EntityID = static_cast<int32_t>(entity);
		verts[1].EntityID = static_cast<int32_t>(entity);
		verts[2].EntityID = static_cast<int32_t>(entity);
		verts[3].EntityID = static_cast<int32_t>(entity);
		verts[4].EntityID = static_cast<int32_t>(entity);
		verts[5].EntityID = static_cast<int32_t>(entity);
		verts[6].EntityID = static_cast<int32_t>(entity);
		verts[7].EntityID = static_cast<int32_t>(entity);

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

		return CreateRef<Mesh>(CreateRef<MeshData>(verts, indices, glm::mat4{ 1.0f }));
	}
}
