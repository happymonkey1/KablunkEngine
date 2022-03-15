#ifndef KABLUNK_SCENE_COMPONENT_H
#define KABLUNK_SCENE_COMPONENT_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define GLM_ENABLE_EXPERIMENTAL // needed for some reason LOL
#include <glm/gtx/quaternion.hpp>

#include "Kablunk/Scripts/NativeScriptEngine.h"
#include "Kablunk/Scripts/NativeScript.h"

#include "Kablunk/Scene/Entity.h"
#include "Kablunk/Renderer/Texture.h"
#include "Kablunk/Scene/SceneCamera.h"
#include "Kablunk/Core/AssetManager.h"
#include "Kablunk/Core/Uuid64.h"
#include "Kablunk/Renderer/Mesh.h"

#include <filesystem>


namespace Kablunk
{
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

		operator glm::mat4 () const { return GetTransform(); }
	};

	struct SpriteRendererComponent
	{
		Asset<Texture2D> Texture{ Asset<Texture2D>("") };
		glm::vec4 Color{ 1.0f };
		float Tiling_factor{ 1.0f };

		glm::vec2 GetTextureDimensions() const 
		{ 
			return { Texture.Get()->GetWidth(), Texture.Get()->GetHeight() }; 
		}

		SpriteRendererComponent() = default;
		SpriteRendererComponent(const SpriteRendererComponent&) = default;
		SpriteRendererComponent(glm::vec4 color) 
			: Color{ color } { }
		SpriteRendererComponent(const Asset<Texture2D>& texture, glm::vec4 color, float tiling_factor = 1.0f) 
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
		Scope<NativeScriptInterface> Instance = nullptr;
		std::filesystem::path Filepath = "";

		using InstantiateScriptFunc = Scope<NativeScriptInterface>(*)();
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
			InstantiateScript	= [args...]() -> Scope<NativeScriptInterface> { return CreateScope<T>(args...) };
		}

		void BindEditor()
		{
			BindEditor(Filepath);
		}

		// #TODO maybe add preprocessor to remove this from runtime builds, only necessary for editor
		void BindEditor(const std::filesystem::path& filepath)
		{
			if (filepath.empty())
				return;

			auto struct_names = Parser::CPP::FindClassAndStructNames(filepath.string(), 1);
			if (struct_names.empty())
			{
				KB_CORE_ERROR("Could not parse struct from file '{0}'", filepath);
				return;
			}
			auto struct_name = struct_names[0];

			Instance = NativeScriptEngine::GetScript(struct_name);

			if (!Instance)
			{
				KB_CORE_ERROR("Script could not be loaded from file '{0}'", filepath);
				return;
			}

			if (Instance)
			{
				Filepath = filepath;

				
				// #TODO Move to scene OnStartRuntime
				try
				{
					Instance->OnAwake();
				}
				catch (std::bad_alloc& e)
				{
					KB_CORE_ERROR("Memery allocation exception '{0}' occurred during OnAwake()", e.what());
					KB_CORE_WARN("Script '{0}' failed! Unloading!", Filepath);
					Instance.reset();
				}
				catch (std::exception& e)
				{
					KB_CORE_ERROR("Generic exception '{0}' occurred during OnAwake()", e.what());
					KB_CORE_WARN("Script '{0}' failed! Unloading!", Filepath);
					Instance.reset();
				}
				catch (...)
				{
					KB_CORE_ERROR("Unkown exception occurred during OnAwake()");
					KB_CORE_WARN("Script '{0}' failed! Unloading!", Filepath);
					Instance.reset();
				}
			}
		}


		friend class Scene;
	};

	struct MeshComponent
	{
		IntrusiveRef<Kablunk::Mesh> Mesh;
		std::string Filepath = "";

		MeshComponent() = default;
		MeshComponent(const IntrusiveRef<Kablunk::Mesh>& mesh)
			: Mesh{ mesh } { }
		MeshComponent(const MeshComponent&) = default;

		void LoadMeshFromFileEditor(const std::string& filepath, Entity entity)
		{
			if (!Filepath.empty())
				Mesh.reset();

			auto mesh_data = IntrusiveRef<MeshData>::Create(filepath, entity);
			Mesh = IntrusiveRef<Kablunk::Mesh>::Create(mesh_data);

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
}

#endif
