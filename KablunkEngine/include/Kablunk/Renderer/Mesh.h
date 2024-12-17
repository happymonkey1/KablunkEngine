#ifndef KABLUNK_RENDERER_MESH_H
#define KABLUNK_RENDERER_MESH_H

#include <glm/glm.hpp>

#include "Kablunk/Core/RefCounting.h"
#include "Kablunk/Core/Timestep.h"
#include "Kablunk/Renderer/backend/buffer.h"
#include "Kablunk/Renderer/backend/texture.h"
#include "Kablunk/Renderer/backend/shader.h"
#include "Kablunk/Renderer/material_asset.h"
#include "Kablunk/Math/vec.hpp"

#include <vector>
#include <string>
#include <unordered_map>

#include "Kablunk/renderer/mesh_handle.h"


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

namespace kb::render
{ // start namespace kb::render
struct vertex_t
{
	vec3_packed Position;
	vec3_packed Normal;
	vec3_packed Tangent;
	vec3_packed Binormal;
	vec2_packed TexCoord;
};

struct animated_vertex_t
{
	vec3_packed Position;
	vec3_packed Normal;
	vec3_packed Tangent;
	vec3_packed Binormal;
	vec2_packed TexCoord;

	uint32_t Ids[4] = { 0, 0, 0, 0 };
	float Weights[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

	void add_bone_data(uint32_t bone_id, float weight)
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

struct bone_info_t
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
	vertex_t V0;
	vertex_t V1;
	vertex_t V2;

	Triangle(const vertex_t& v0, const vertex_t& v1, const vertex_t& v2)
		: V0{ v0 }, V1{ v1 }, V2{ v2 }
	{ }
};

struct sub_mesh_t
{
    u32 BaseVertex;
    u32 BaseIndex;
    u32 Material_index;
    u32 IndexCount;
    u32 VertexCount;

	glm::mat4 Transform{ 1.0f };
	glm::mat4 Local_transform{ 1.0f };

	std::string node_name, mesh_name;
};

class MeshData : public RefCounted
{
public:
	MeshData(const std::string& filename, kb::Entity entity);
	MeshData(
        mesh_handle p_handle,
        const std::vector<vertex_t>& p_vertices,
        const std::vector<Index>& indices,
        const glm::mat4& transform
    );
	virtual ~MeshData() override;

    auto get_handle() const noexcept -> mesh_handle { return m_handle; }

	const std::vector<vertex_t>& get_vertices() const { return m_static_vertices; }
	const std::vector<Index>& get_indices() const { return m_indices; }
	arc<backend::shader> get_shader() { return m_mesh_shader; }
	arc<backend::vertex_buffer> get_vertex_buffer() const { return m_vertex_buffer; }
	arc<backend::index_buffer> get_index_buffer() const { return m_index_buffer; }

	std::vector<arc<material_asset>>& get_materials() { return m_materials; }
	const std::vector<arc<material_asset>>& get_materials() const { return m_materials; }

	const std::vector<arc<backend::texture_2d>>& get_textures() const { return m_textures; }
	const std::vector<arc<backend::texture_2d>>& get_normal_maps() const { return m_normal_map; }
	const std::string& get_filepath() const { return m_filepath; }

	void set_sub_meshes(const std::vector<sub_mesh_t>& p_sub_meshes);
	std::vector<sub_mesh_t>& get_sub_meshes() { return m_sub_meshes; }
	const std::vector<sub_mesh_t>& get_sub_meshes() const { return m_sub_meshes; }

	const std::vector<Triangle>& get_triangle_cache(u32 index) const { return m_triangle_cache.at(index); }
    auto get_triangle_count() const noexcept -> size_t { return m_triangle_count; }

	const aiNodeAnim* FindNodeAnim(const aiAnimation* animation, const std::string& node_name);
    u32 FindPosition(float animation_time, const aiNodeAnim* root_node_anim);
    u32 FindRotation(float animation_time, const aiNodeAnim* root_node_anim);
    u32 FindScaling(float animation_time, const aiNodeAnim* root_node_anim);
	glm::vec3 InterpolateTranslation(float animation_time, const aiNodeAnim* node_anim);
	glm::quat InterpolateRotation(float animation_time, const aiNodeAnim* node_anim);
	glm::vec3 InterpolateScale(float animation_time, const aiNodeAnim* node_anim);

	void ReadNodeHierarchy(float animation_time, const aiNode* root, const glm::mat4& parent_transform);

private:
	void TraverseNodes(aiNode* root, const glm::mat4& parent_transform = glm::mat4{ 1.0f }, u32 level = 0);

private:
	box<Assimp::Importer> m_importer;

	arc<backend::vertex_buffer> m_vertex_buffer;
	arc<backend::index_buffer> m_index_buffer;

	std::vector<vertex_t> m_static_vertices;
	std::vector<animated_vertex_t> m_animated_vertices;
	std::vector<Index> m_indices;
	unordered_flat_map<std::string, u32> m_bone_mapping;
	unordered_flat_map<aiNode*, std::vector<u32>> m_node_map;

    u32 m_bone_count = 0;
	std::vector<bone_info_t> m_bone_info;
	std::vector<sub_mesh_t> m_sub_meshes;

	const aiScene* m_scene;

	glm::mat4 m_inverse_transform{ 1.0f };

	arc<backend::shader> m_mesh_shader;
	std::vector<arc<backend::texture_2d>> m_textures;
	std::vector<arc<backend::texture_2d>> m_normal_map;
	std::vector<arc<material_asset>> m_materials;

	unordered_flat_map<u32, std::vector<Triangle>> m_triangle_cache;
    size_t m_triangle_count = 0;

	std::string m_filepath;
    mesh_handle m_handle{};

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
	Mesh(arc<MeshData> mesh_data);
	Mesh(const arc<Mesh>& other);
	Mesh(arc<MeshData> mesh_data, const std::vector<u32>& sub_meshes);
	virtual ~Mesh() noexcept = default;

    auto get_handle() const noexcept -> mesh_handle { return m_mesh_data->get_handle(); }

	void OnUpdate(Timestep ts);

	std::vector<u32>& GetSubmeshes() { return m_submeshes; }
	const std::vector<u32>& GetSubmeshes() const { return m_submeshes; }

	void set_sub_meshes(const std::vector<u32>& p_sub_meshes);

	arc<MeshData> GetMeshData() { return m_mesh_data; }
	arc<MeshData> GetMeshData() const { return m_mesh_data; }
	void SetMeshData(arc<MeshData> mesh_data) { m_mesh_data = mesh_data; }

	arc<material_table>& get_material_table() { return m_material_table; }
	const arc<material_table>& get_material_table() const { return m_material_table; }

private:
    auto init_material_table(const std::vector<arc<material_asset>>& p_materials) noexcept -> void;

private:
	arc<MeshData> m_mesh_data;
	std::vector<u32> m_submeshes;

	arc<material_table> m_material_table;
};

// #TODO move elsewhere
class MeshFactory
{
public:
    // Kablunk resource notation (krn) used to construct the mesh handle
    inline static constexpr const char* k_cube_mesh_krn = "kb::mesh::cube";
    inline static constexpr mesh_handle k_cube_mesh_handle = mesh_handle::into(std::string_view{ k_cube_mesh_krn });
public:
	static arc<Mesh> CreateCube(float side_length, Entity entity);
};
} // end namespace kb::render

#endif
