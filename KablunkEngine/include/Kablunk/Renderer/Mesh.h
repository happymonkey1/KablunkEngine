#ifndef KABLUNK_RENDERER_MESH_H
#define KABLUNK_RENDERER_MESH_H

#include <glm/glm.hpp>

#include "Kablunk/Core/RefCounting.h"
#include "Kablunk/Core/Timestep.h"
#include "Kablunk/Renderer/Buffer.h"
#include "Kablunk/Renderer/Texture.h"
#include "Kablunk/Renderer/Shader.h"
#include "Kablunk/Renderer/VertexArray.h"
#include "Kablunk/Renderer/MaterialAsset.h"

#include <vector>
#include <string>
#include <unordered_map>


// Forward decs
struct aiScene;
struct aiNode;
struct aiNodeAnim;
struct aiAnimation;

namespace Assimp
{
	class Importer;
}

namespace kb
{
	class Entity;
}

namespace kb
{
	struct Vertex
	{
		glm::vec3 Position;
		glm::vec3 Normal;
		glm::vec3 Tangent;
		glm::vec3 Binormal;
		glm::vec2 TexCoord;
	};

	struct AnimatedVertex
	{
		glm::vec3 Position;
		glm::vec3 Normal;
		glm::vec3 Tangent;
		glm::vec3 Binormal;
		glm::vec2 TexCoord;

		uint32_t Ids[4] = { 0, 0, 0, 0 };
		float Weights[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

		void AddBoneData(uint32_t bone_id, float weight)
		{
			for (size_t i = 0; i < 4; ++i)
			{
				if (Weights[i] == 0.0)
				{
					Ids[i] = bone_id;
					Weights[i] = weight;
					return;
				}
			}

			KB_CORE_WARN("Vertex has more than four bones or weights affecting it, extra data is discarded!");
		}

		int32_t EntityID;
	};

	struct BoneInfo
	{
		glm::mat4 Bone_offset;
		glm::mat4 Final_transformation;
	};

	struct VertexBoneData
	{
		uint32_t Ids[4];
		float Weights[4];

		VertexBoneData() : Ids{ 0 }, Weights{ 0.0f } {}

		void AddBoneData(uint32_t bone_id, float weight)
		{
			for (size_t i = 0; i < 4; ++i)
			{
				if (Weights[i] == 0.0)
				{
					Ids[i] = bone_id;
					Weights[i] = weight;
					return;
				}
			}

			KB_CORE_WARN("Vertex has more than four bones or weights affecting it, extra data is discarded!");
		}
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

	class Submesh
	{
	public:
		uint32_t BaseVertex;
		uint32_t BaseIndex;
		uint32_t Material_index;
		uint32_t IndexCount;
		uint32_t VertexCount;

		glm::mat4 Transform{ 1.0f };
		glm::mat4 Local_transform{ 1.0f };

		std::string node_name, mesh_name;
	};

	class MeshData : public RefCounted
	{
	public:
		MeshData(const std::string& filename, kb::Entity entity);
		MeshData(const std::vector<Vertex>& verticies, const std::vector<Index>& indices, const glm::mat4& transform);
		virtual ~MeshData();

		const std::vector<Vertex>& GetVertices() const { return m_static_vertices; }
		const std::vector<Index>& GetIndicies() const { return m_indices; }
		ref<Shader> GetShader() { return m_mesh_shader; }
		ref<VertexBuffer> GetVertexBuffer() const { return m_vertex_buffer; }
		ref<IndexBuffer> GetIndexBuffer() const { return m_index_buffer; }

		std::vector<ref<Material>>& GetMaterials() { return m_materials; }
		const std::vector<ref<Material>> GetMaterials() const { return m_materials; }

		const std::vector<ref<Texture2D>> GetTextures() const { return m_textures; }
		const std::vector<ref<Texture2D>> GetNormalMaps() const { return m_normal_map; }
		const std::string& GetFilepath() const { return m_filepath; }

		void SetSubmeshes(const std::vector<Submesh>& submeshes);
		std::vector<Submesh>& GetSubmeshes() { return m_sub_meshes; }
		const std::vector<Submesh>& GetSubmeshes() const { return m_sub_meshes; }

		const std::vector<Triangle>& GetTriangleCache(uint32_t index) const { return m_triangle_cache.at(index); }

		const aiNodeAnim* FindNodeAnim(const aiAnimation* animation, const std::string& node_name);
		uint32_t FindPosition(float animation_time, const aiNodeAnim* root_node_anim);
		uint32_t FindRotation(float animation_time, const aiNodeAnim* root_node_anim);
		uint32_t FindScaling(float animation_time, const aiNodeAnim* root_node_anim);
		glm::vec3 InterpolateTranslation(float animation_time, const aiNodeAnim* node_anim);
		glm::quat InterpolateRotation(float animation_time, const aiNodeAnim* node_anim);
		glm::vec3 InterpolateScale(float animation_time, const aiNodeAnim* node_anim);

		void ReadNodeHierarchy(float animation_time, const aiNode* root, const glm::mat4& parent_transform);
	private:
		void TraverseNodes(aiNode* root, const glm::mat4& parent_transform = glm::mat4{ 1.0f }, uint32_t level = 0);
	private:
		box<Assimp::Importer> m_importer;

		ref<VertexBuffer> m_vertex_buffer;
		ref<IndexBuffer> m_index_buffer;

		std::vector<Vertex> m_static_vertices;
		std::vector<AnimatedVertex> m_animated_vertices;
		std::vector<Index> m_indices;
		kb::unordered_flat_map<std::string, uint32_t> m_bone_mapping;
		kb::unordered_flat_map<aiNode*, std::vector<uint32_t>> m_node_map;

		uint32_t m_bone_count = 0;
		std::vector<BoneInfo> m_bone_info;
		std::vector<Submesh> m_sub_meshes;
	
		const aiScene* m_scene;

		glm::mat4 m_inverse_transform{ 1.0f };

		ref<Shader> m_mesh_shader;
		std::vector<ref<Texture2D>> m_textures;
		std::vector<ref<Texture2D>> m_normal_map;
		std::vector<ref<Material>> m_materials;
		
		kb::unordered_flat_map<uint32_t, std::vector<Triangle>> m_triangle_cache;

		std::string m_filepath;

		// Animation
		bool m_is_animated = false;
		float m_animation_time = 0.0f;
		float m_world_time = 0.0f;
		float m_time_multiplier = 1.0f;
		float m_animation_playing = true;

		friend class Renderer;
	};


	class Mesh : public RefCounted
	{
	public:
		Mesh(ref<MeshData> mesh_data);
		Mesh(const ref<Mesh>& other);
		Mesh(ref<MeshData> mesh_data, const std::vector<uint32_t>& submeshes);
		virtual ~Mesh();

		void OnUpdate(Timestep ts);

		std::vector<uint32_t>& GetSubmeshes() { return m_submeshes; }
		const std::vector<uint32_t>& GetSubmeshes() const { return m_submeshes; }

		void SetSubmeshes(const std::vector<uint32_t>& submeshes);

		ref<MeshData> GetMeshData() { return m_mesh_data; }
		ref<MeshData> GetMeshData() const { return m_mesh_data; }
		void SetMeshData(ref<MeshData> mesh_data) { m_mesh_data = mesh_data; }

		ref<MaterialTable>& GetMaterials() { return m_material_table; }
		const ref<MaterialTable>& GetMaterials() const { return m_material_table; }
	private:
		ref<MeshData> m_mesh_data;
		std::vector<uint32_t> m_submeshes;

		ref<MaterialTable> m_material_table;
	};

	// #TODO move elsewhere
	class MeshFactory
	{
	public:
		static ref<Mesh> CreateCube(float side_length, Entity entity);
	};
}

#endif
