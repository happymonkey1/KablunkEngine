#ifndef KABLUNK_RENDERER_MESH_H
#define KABLUNK_RENDERER_MESH_H

#include <glm/glm.hpp>

#include "Kablunk/Core/Timestep.h"
#include "Kablunk/Renderer/Buffer.h"
#include "Kablunk/Renderer/Texture.h"
#include "Kablunk/Renderer/Shader.h"
#include "Kablunk/Renderer/VertexArray.h"

#include <vector>
#include <string>
#include <unordered_map>


// Forward decs
struct aiScene;

namespace Assimp
{
	class Importer;
}

namespace Kablunk
{
	struct Vertex
	{
		glm::vec3 Position;
		glm::vec3 Normal;
		glm::vec3 Tangent;
		glm::vec3 Binormal;
		glm::vec2 TexCoord;
	};

	struct Index
	{
		uint32_t V1;
		uint32_t V2;
		uint32_t V3;
	};

	struct Triangle
	{
		Vertex V0;
		Vertex V1;
		Vertex V2;

		Triangle(const Vertex& v0, const Vertex& v1, const Vertex& v2)
			: V0{ v0 }, V1{ v1 }, V2{ v2 } 
		{ }
	};

	class MeshData
	{
	public:
		MeshData(const std::string& filename);
		MeshData(const std::vector<Vertex>& verticies, const std::vector<Index>& indices, const glm::mat4& transform);
		virtual ~MeshData();

		const std::vector<Vertex>& GetVertices() const { return m_vertices; }
		const std::vector<Index>& GetIndicies() const { return m_indices; }
		Ref<Shader> GetShader() { return m_mesh_shader; }
		Ref<VertexBuffer> GetVertexBuffer() const { return m_vertex_buffer; }
		Ref<IndexBuffer> GetIndexBuffer() const { return m_index_buffer; }
		const BufferLayout& GetBufferLayout() const { return m_vertex_buffer_layout; }

		const std::vector<Ref<Texture2D>> GetTextures() const { return m_textures; }
		const std::vector<Ref<Texture2D>> GetNormalMap() const { return m_normal_map; }
		const std::string& GetFilepath() const { return m_filepath; }

		const std::vector<Triangle>& GetTriangleCache(uint32_t index) const { return m_triangle_cache.at(index); }

		void BindVertexBuffer() { KB_CORE_ERROR("MeshData BindVertexBuffer() not implemented!"); };
	private:
		Scope<Assimp::Importer> m_importer;

		Ref<VertexBuffer> m_vertex_buffer;
		Ref<IndexBuffer> m_index_buffer;
		BufferLayout m_vertex_buffer_layout;

		std::vector<Vertex> m_vertices;
		std::vector<Index> m_indices;
	
		const aiScene* m_scene;

		Ref<Shader> m_mesh_shader;
		std::vector<Ref<Texture2D>> m_textures;
		std::vector<Ref<Texture2D>> m_normal_map;
		
		std::unordered_map<uint32_t, std::vector<Triangle>> m_triangle_cache;

		std::string m_filepath;

		friend class Renderer;
	};


	class Mesh
	{
	public:
		Mesh(Ref<MeshData> mesh_data);
		Mesh(const Ref<Mesh>& other);
		virtual ~Mesh();

		void OnUpdate(Timestep ts);

		Ref<MeshData> GetMeshData() { return m_mesh_data; }
		Ref<MeshData> GetMeshData() const { return m_mesh_data; }
		Ref<VertexArray> GetVertexArray() const { return m_vertex_array; }
		void SetMeshData(Ref<MeshData> mesh_data) { m_mesh_data = mesh_data; }
	private:
		Ref<MeshData> m_mesh_data;
		Ref<VertexArray> m_vertex_array;
	};

	// #TODO move elsewhere
	class MeshFactory
	{
	public:
		static Ref<Mesh> CreateCube(float side_length);
	};
}

#endif
