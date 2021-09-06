#include "kablunkpch.h"

#include "Kablunk/Core/Core.h"
#include "Kablunk/Renderer/Mesh.h"
#include "Kablunk/Renderer/Shader.h"
#include "Kablunk/Renderer/Renderer.h"

#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>

namespace Kablunk
{
	static constexpr const uint32_t s_mesh_import_flags =
		aiProcess_Triangulate | aiProcess_GenNormals | aiProcess_CalcTangentSpace | aiProcess_GenUVCoords | aiProcess_ValidateDataStructure;
	

	MeshData::MeshData(const std::string& filepath)
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

		m_mesh_shader = Renderer::GetShaderLibrary()->Get("Kablunk_diffuse_static");

		// #TODO submeshes
		aiMesh* mesh = scene->mMeshes[0];
		size_t vertex_count = mesh->mNumVertices;

		// ---Vertices---
		// #TODO 3d animations
		{
			for (size_t i = 0; i < vertex_count; ++i)
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
					v.TexCoord = { mesh->mTextureCoords[0]->x, mesh->mTextureCoords[0]->y };
				}

				m_vertices.push_back(v);
			}
		}

		// ---Indices---
		// #TODO 3d animations
		{
			for (size_t i = 0; i < mesh->mNumFaces; ++i)
			{
				KB_CORE_ASSERT(mesh->mFaces[i].mNumIndices == 3, "Must have 3 indices");
				Index index = { mesh->mFaces[i].mIndices[0], mesh->mFaces[i].mIndices[1], mesh->mFaces[i].mIndices[2] };
				m_indices.push_back(index);

				m_triangle_cache[i].emplace_back(m_vertices[index.V1], m_vertices[index.V2], m_vertices[index.V3]);
			}
		}

		// #TODO 3d animations
		{
			m_vertex_buffer = VertexBuffer::Create((uint32_t)m_vertices.size());
			m_vertex_buffer->SetData(m_vertices.data(), (uint32_t)m_vertices.size());
			m_vertex_buffer_layout = {
				{ ShaderDataType::Float3, "a_Position" },
				{ ShaderDataType::Float3, "a_Normal" },
				{ ShaderDataType::Float3, "a_Tangent" },
				{ ShaderDataType::Float3, "a_Binormal" },
				{ ShaderDataType::Float2, "a_TexCoord" }
			};
			m_vertex_buffer->SetLayout(m_vertex_buffer_layout);
		}

		m_index_buffer = IndexBuffer::Create(m_indices.data(), (uint32_t)(m_indices.size() * sizeof(Index)));
	}

	MeshData::MeshData(const std::vector<Vertex>& verticies, const std::vector<Index>& indices, const glm::mat4& transform)
		: m_vertices{ verticies }, m_indices{ indices }
	{
		m_mesh_shader = Renderer::GetShaderLibrary()->Get("Kablunk_diffuse_static");

		m_vertex_buffer = VertexBuffer::Create((uint32_t)m_vertices.size());

		m_vertex_buffer_layout = {
			{ ShaderDataType::Float3, "a_Position" },
			{ ShaderDataType::Float3, "a_Normal" },
			{ ShaderDataType::Float3, "a_Tangent" },
			{ ShaderDataType::Float3, "a_Binormal" },
			{ ShaderDataType::Float2, "a_TexCoord" }
		};
		m_vertex_buffer->SetLayout(m_vertex_buffer_layout);
		m_vertex_buffer->SetData(m_vertices.data(), (uint32_t)m_vertices.size());

		m_index_buffer = IndexBuffer::Create(m_indices.data(), (uint32_t)(m_indices.size() * sizeof(Index)));
	}

	MeshData::~MeshData()
	{

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

	Ref<Mesh> MeshFactory::CreateCube(float side_length)
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
		verts[0].Binormal = { -1.0f, -1.0f,  1.0f };
		verts[1].Binormal = { 1.0f, -1.0f,  1.0f };
		verts[2].Binormal = { 1.0f,  1.0f,  1.0f };
		verts[3].Binormal = { -1.0f,  1.0f,  1.0f };
		verts[4].Binormal = { -1.0f, -1.0f, -1.0f };
		verts[5].Binormal = { 1.0f, -1.0f, -1.0f };
		verts[6].Binormal = { 1.0f,  1.0f, -1.0f };
		verts[7].Binormal = { -1.0f,  1.0f, -1.0f };

		// #FIXME this is garbage data
		verts[0].Tangent = { -1.0f, -1.0f,  1.0f };
		verts[1].Tangent = { 1.0f, -1.0f,  1.0f };
		verts[2].Tangent = { 1.0f,  1.0f,  1.0f };
		verts[3].Tangent = { -1.0f,  1.0f,  1.0f };
		verts[4].Tangent = { -1.0f, -1.0f, -1.0f };
		verts[5].Tangent = { 1.0f, -1.0f, -1.0f };
		verts[6].Tangent = { 1.0f,  1.0f, -1.0f };
		verts[7].Tangent = { -1.0f,  1.0f, -1.0f };

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
		indices[0]  = { 0, 1, 2 };
		indices[1]  = { 2, 3, 0 };
		indices[2]  = { 1, 5, 6 };
		indices[3]  = { 6, 2, 1 };
		indices[4]  = { 7, 6, 5 };
		indices[5]  = { 5, 4, 7 };
		indices[6]  = { 4, 0, 3 };
		indices[7]  = { 3, 7, 4 };
		indices[8]  = { 4, 5, 1 };
		indices[9]  = { 1, 0, 4 };
		indices[10] = { 3, 2, 6 };
		indices[11] = { 6, 7, 3 };

		return CreateRef<Mesh>(CreateRef<MeshData>(verts, indices, glm::mat4{ 1.0f }));
	}
}
