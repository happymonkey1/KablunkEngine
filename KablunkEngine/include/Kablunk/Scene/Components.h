#ifndef KABLUNK_SCENE_COMPONENT_H
#define KABLUNK_SCENE_COMPONENT_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define GLM_ENABLE_EXPERIMENTAL // needed for some reason LOL
#include <glm/gtx/quaternion.hpp>

#include "Kablunk/Core/Uuid64.h"

#include "Kablunk/Scripts/NativeScriptEngine.h"
#include "Kablunk/Scripts/NativeScript.h"

#include "Kablunk/Scene/Entity.h"
#include "Kablunk/Scene/SceneCamera.h"

#include "Kablunk/Renderer/Texture.h"
#include "Kablunk/Renderer/Mesh.h"
#include "Kablunk/Renderer/MaterialAsset.h"
#include "Kablunk/Renderer/RenderCommand.h"

#include "Kablunk/Project/ProjectManager.h"

#include "Kablunk/Asset/AssetCommand.h"

#include "Kablunk/UI/PanelFactory.h"

#include "Kablunk/Utilities/Parser.h"

#include <filesystem>


namespace kb
{ // start namespace Kablunk

struct IdComponent
{
	uuid::uuid64 Id{ uuid::generate() };

	IdComponent() = default;
	IdComponent(const IdComponent&) = default;
	IdComponent(uuid::uuid64 id) : Id{ id } { }

	operator uint64_t& () { return Id; }
	operator const uint64_t& () const { return Id; }
};

struct TagComponent
{
	std::string Tag = "";
		
	TagComponent() = default;
	TagComponent(const TagComponent&) = default;
	TagComponent(const std::string& tag) : Tag{ tag } { }

	operator std::string&()				{ return Tag; }
	operator const std::string&() const	{ return Tag; }
};

struct ParentingComponent
{
	uuid::uuid64 Parent = uuid::nil_uuid;
	std::vector<uuid::uuid64> Children = std::vector<uuid::uuid64>{};

	ParentingComponent() = default;
	ParentingComponent(const ParentingComponent&) = default;
	ParentingComponent(uuid::uuid64 parent) : Parent{ parent } {}
};

struct PrefabComponent
{
	uuid::uuid64 Prefab_id = uuid::nil_uuid;
	uuid::uuid64 Entity_id = uuid::nil_uuid;

	PrefabComponent() = default;
	PrefabComponent(uuid::uuid64 p_prefab_id, uuid::uuid64 p_entity_id)
		: Prefab_id{ p_prefab_id }, Entity_id{ p_entity_id }
	{ }
	PrefabComponent(const PrefabComponent&) = default;
};

struct TransformComponent
{
	glm::vec3 Translation	= glm::vec3{ 0.0f };
	glm::vec3 Rotation		= glm::vec3{ 0.0f };
	glm::vec3 Scale			= glm::vec3{ 1.0f };

	TransformComponent() = default;
	TransformComponent(const TransformComponent&) = default;
	TransformComponent(const glm::vec3& translation) : Translation{ translation } { }

	glm::mat4 GetTransform() const
	{
		glm::mat4 rotation = glm::toMat4(glm::quat(Rotation));

		return glm::translate(glm::mat4{ 1.0f }, Translation)
			* rotation
			* glm::scale(glm::mat4{ 1.0f }, Scale);
	}

	operator glm::mat4() const { return GetTransform(); }
};

struct SpriteRendererComponent
{
	// #TODO old "asset" class uses absolute path. 
	asset::asset_id_t Texture{ asset::null_asset_id };
	glm::vec4 Color{ 1.0f };
	float Tiling_factor{ 1.0f };
	// #TODO should this be entity wide, instead of just on SpriteRenderers?
	bool Visible = true;

	glm::vec2 GetTextureDimensions() const 
	{ 
        const auto& texture_asset = asset::get_asset<Texture2D>(Texture);
        return glm::vec2{ texture_asset->GetWidth(), texture_asset->GetHeight() }; 
	}

	void SetVisible(bool v) { Visible = v; }
	bool GetVisible() const { return Visible; }

	SpriteRendererComponent() = default;
	SpriteRendererComponent(const SpriteRendererComponent&) = default;
	SpriteRendererComponent(glm::vec4 color) 
		: Color{ color } { }
	SpriteRendererComponent(const ref<Texture2D>& texture, glm::vec4 color, float tiling_factor = 1.0f) 
		: Texture{ texture }, Color{ color }, Tiling_factor{ tiling_factor } { }
};

struct CircleRendererComponent
{
	glm::vec4 Color{ 1.0f, 1.0f, 1.0f, 1.0f };
	float Radius{ 0.5f };
	float Thickness = 1.0f;
	float Fade = 0.005f;

	CircleRendererComponent() = default;
	CircleRendererComponent(const CircleRendererComponent&) = default;
	CircleRendererComponent(const glm::vec4& color, float radius, float thickness, float fade)
		: Color{ color }, Radius{ radius }, Thickness{ thickness }, Fade{ fade } {}
};

struct CameraComponent
{
	SceneCamera Camera;
	bool Primary{ true };
	bool Fixed_aspect_ratio = false;

	CameraComponent() = default;
	CameraComponent(const CameraComponent& projection) = default;
};

// #TODO allow for multiple scripts to be attached to the same entity
struct NativeScriptComponent
{
	std::unique_ptr<INativeScript> Instance = nullptr;
	std::filesystem::path Filepath = "";

	using InstantiateScriptFunc = Scope<INativeScript>(*)();
	// Function pointer instead of std::function bc of potential memory allocations
	InstantiateScriptFunc InstantiateScript = nullptr;

	NativeScriptComponent() = default;
	NativeScriptComponent(const NativeScriptComponent& other)
		: Instance{nullptr}, Filepath{other.Filepath}
	{

	}

	NativeScriptComponent(NativeScriptComponent&& other) noexcept
	{
		Instance = std::move(other.Instance);
		Filepath = std::move(other.Filepath);

		other.Instance = nullptr;
		other.Filepath = "";
	}

	~NativeScriptComponent() noexcept
	{
		if (Instance)
			destroy_script();
	}
		
	NativeScriptComponent& operator=(const NativeScriptComponent& other)
	{
		Instance = nullptr;
		Filepath = other.Filepath;

		return *this;
	}

	NativeScriptComponent& operator=(NativeScriptComponent&& other) noexcept
	{
		Instance = std::move(other.Instance);
		Filepath = other.Filepath;
		//BindEditor();

		other.Instance = nullptr;
		other.Filepath = "";

		return *this;
	}

	std::string GetFilepath() const { return Filepath.string(); }

	// Runtime binding
	template <typename T, typename... Args>
	void BindRuntime(Args... args)
	{
		InstantiateScript = [args...]() -> Scope<NativeScript> { return CreateScope<T>(args...); };
	}

	void BindEditor()
	{
		BindEditor(Filepath);
	}

	// #TODO maybe add preprocessor to remove this from runtime builds, only necessary for editor
	void BindEditor(const std::filesystem::path& filepath)
	{
		if (filepath.empty())
		{
			KB_CORE_ERROR("Empty filepath passed to BindEditor()!");
			return;
		}

		KB_CORE_ASSERT(ProjectManager::get().get_active(), "no active project!");
		std::filesystem::path absolute_path = ProjectManager::get().get_active()->get_project_directory() / filepath;

		auto annotations = Parser::CPP::FindParserTokens(absolute_path.string());
		for (const auto& annot : annotations)
			KB_CORE_INFO("{}", annot.ToString());

		auto struct_names = Parser::CPP::FindClassAndStructNames(absolute_path.string(), 1);
		if (struct_names.empty())
		{
			KB_CORE_ERROR("Could not parse struct from file '{0}'", filepath);
			return;
		}
		auto struct_name = struct_names[0];

		Instance = NativeScriptEngine::get().get_script(struct_name);

		if (!Instance)
		{
			KB_CORE_ERROR("Script could not be loaded from file '{0}'", absolute_path);
			return;
		}

		if (Instance)
			Filepath = filepath;
	}

	// Destroy the instance of the script and potentially free memory.
	void destroy_script(bool free_memory = true)
	{
		Instance.reset();
	}

	friend class Scene;
};

struct MeshComponent
{
	ref<kb::Mesh> Mesh;
	ref<kb::MaterialTable> Material_table = ref<kb::MaterialTable>::Create();
	std::string Filepath = "";

	MeshComponent() = default;
	MeshComponent(const ref<kb::Mesh>& mesh)
		: Mesh{ mesh } { }
	MeshComponent(const MeshComponent& other)
		: Mesh{ other.Mesh }, Material_table{ ref<kb::MaterialTable>::Create(other.Material_table) } {};

	void LoadMeshFromFileEditor(const std::string& filepath, Entity entity)
	{
		if (!Filepath.empty())
			Mesh.reset();

		auto mesh_data = ref<MeshData>::Create(filepath, entity);
		Mesh = ref<kb::Mesh>::Create(mesh_data);

		Filepath = filepath;
	}
};

struct PointLightComponent
{
	float Multiplier;
	glm::vec3 Radiance;
	float Radius;
	float Min_radius;
	float Falloff;

	PointLightComponent() = default;
	PointLightComponent(float multiplier, const glm::vec3& radiance, float radius, float min_radius, float falloff)
		: Multiplier{ multiplier }, Radiance{ radiance }, Radius{ radius }, Min_radius{ min_radius }, Falloff{ falloff } {}
	PointLightComponent(const PointLightComponent&) = default;
};

struct RigidBody2DComponent
{
	enum class RigidBodyType { Static = 0, Dynamic, Kinematic };

	RigidBodyType Type = RigidBodyType::Static;
	bool Fixed_rotation = false;
	bool Does_gravity_affect = true;

	void* Runtime_body = nullptr;

	RigidBody2DComponent() = default;
	RigidBody2DComponent(const RigidBody2DComponent&) = default;
};

struct BoxCollider2DComponent
{
	glm::vec2 Offset = { 0.0f, 0.0f };
	glm::vec2 Size = { 0.5f, 0.5f };

	// #TODO move to physics material
	float Density = 1.0f;
	float Friction = 0.5f;
	float Restitution = 0.0f;
	float Restitution_threshold = 0.5f;

	void* Runtime_ficture = nullptr;

	BoxCollider2DComponent() = default;
	BoxCollider2DComponent(const BoxCollider2DComponent&) = default;
};

struct CircleCollider2DComponent
{
	glm::vec2 Offset = { 0.0f, 0.0f };
	float Radius = 0.5f;

	// #TODO move to physics material
	float Density = 1.0f;
	float Friction = 0.5f;
	float Restitution = 0.0f;
	float Restitution_threshold = 0.5f;

	void* Runtime_ficture = nullptr;

	CircleCollider2DComponent() = default;
	CircleCollider2DComponent(const CircleCollider2DComponent&) = default;
};

struct CSharpScriptComponent
{
	std::string Module_name;

	CSharpScriptComponent() = default;
	CSharpScriptComponent(const CSharpScriptComponent&) = default;
	CSharpScriptComponent(const std::string& name) : Module_name{ name } {}
};

struct SceneComponent
{
	uuid::uuid64 Scene_id;
};

struct UIPanelComponent
{
	ui::panel_type_t panel_type = ui::panel_type_t::NONE;
	ui::IPanel* panel = nullptr;

	UIPanelComponent() = default;
	UIPanelComponent(const UIPanelComponent& other)
	{
		panel = ui::PanelFactory::copy_panel(other.panel);

	}
	UIPanelComponent(ui::IPanel* p) : panel{ p } {}
	~UIPanelComponent()
	{
		delete panel;
		panel = nullptr;
	}
};

struct TextComponent
{
    // text string that will be rendered to the screen
    std::string m_text_str = "";
    // asset id for the font to use to render text
    std::string m_font_filename = "Roboto-Regular.ttf";
    // tinting color
    glm::vec4 m_tint_color = glm::vec4{ 1.0f };

    // default constructor
    TextComponent() = default;

    // copy constructor
    TextComponent(const TextComponent& other)
        : m_text_str{ other.m_text_str }
    { }

    // default destructor
    ~TextComponent() = default;
};

} // end namespace Kablunk

#endif
