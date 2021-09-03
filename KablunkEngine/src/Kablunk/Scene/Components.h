#ifndef KABLUNK_SCENE_COMPONENT_H
#define KABLUNK_SCENE_COMPONENT_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define GLM_ENABLE_EXPERIMENTAL // needed for some reason LOL
#include <glm/gtx/quaternion.hpp>

#include "Kablunk/Scripts/NativeScriptEngine.h"
#include "Kablunk/Scripts/NativeScript.h"
#include "Kablunk/Renderer/Texture.h"
#include "Kablunk/Scene/SceneCamera.h"
#include "Kablunk/Core/AssetManager.h"
#include "Kablunk/Core/Uuid64.h"
#include "Kablunk/Renderer/Mesh.h"


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
		std::string Tag;
		

		TagComponent() = default;
		TagComponent(const TagComponent&) = default;
		TagComponent(const std::string& tag) : Tag{ tag } { }

		operator std::string&()				{ return Tag; }
		operator const std::string&() const	{ return Tag; }
	};

	// #TODO make children transforms relative to parents
	struct ParentEntityComponent
	{
		// #TODO should probably switch to a hashmap or something faster
		std::vector<EntityHandle> Child_entities_handles;

		ParentEntityComponent() = default;
		ParentEntityComponent(const ParentEntityComponent&) = default;
		ParentEntityComponent(std::initializer_list<EntityHandle> l) : Child_entities_handles{ l } { }
		
		void AddChildHandle(EntityHandle child_handle) { Child_entities_handles.push_back(child_handle); }
		void AddChildrenHandles(std::initializer_list<EntityHandle> l) { Child_entities_handles.insert(Child_entities_handles.end(), l.begin(), l.end()); }
		void RemoveChildHandle(EntityHandle child_handle) { Child_entities_handles.erase(std::find(Child_entities_handles.begin(), Child_entities_handles.end(), child_handle)); }
		bool ContainsHandle(EntityHandle child_handle) { return std::find(Child_entities_handles.begin(), Child_entities_handles.end(), child_handle) != Child_entities_handles.end(); }

		std::vector<EntityHandle>::const_iterator begin() const { return Child_entities_handles.begin(); }
		std::vector<EntityHandle>::const_iterator end() const { return Child_entities_handles.end(); }
		//std::vector<Entity*>::iterator begin() { return Child_entities_ptrs.begin(); }
		//std::vector<Entity*>::iterator end() { return Child_entities_ptrs.end(); }
	};

	struct ChildEntityComponent
	{
		EntityHandle Parent_entity_handle{ null_entity };

		ChildEntityComponent() = default;
		ChildEntityComponent(const ChildEntityComponent&) = default;

		bool HasParent() const { return Parent_entity_handle != null_entity; }
		void SetParent(EntityHandle parent_handle) { Parent_entity_handle = parent_handle; }
		EntityHandle GetParent() const { return Parent_entity_handle; }
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

		SpriteRendererComponent() = default;
		SpriteRendererComponent(const SpriteRendererComponent&) = default;
		SpriteRendererComponent(glm::vec4 color) 
			: Color{ color } { }
		SpriteRendererComponent(const Asset<Texture2D>& texture, glm::vec4 color, float tiling_factor = 1.0f) 
			: Texture{ texture }, Color{ color }, Tiling_factor{ tiling_factor } { }
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
		Scope<NativeScript> Instance{ nullptr };
		std::string Filepath{ "" };

		using InstantiateScriptFunc = Scope<NativeScript>(*)();
		// Function pointer instead of std::function bc of potential memory allocations
		InstantiateScriptFunc InstantiateScript;

		// Runtime binding
		template <typename T, typename... Args>
		void BindRuntime(Args... args)
		{
			InstantiateScript	= [args...]() -> Scope<NativeScript> { return CreateScope<T>(args...) };
		}

		// #TODO maybe add preprocessor to remove this from runtime builds, only necessary for editor
		void BindEditor(const std::string& filepath, Entity entity)
		{
			if (filepath.empty())
				return;

			auto struct_names = Parser::CPP::FindStructNames(filepath, 1);
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

			// #TODO might not be necessary, i don't know if setting new ptr cleans up old reference
			if (!Filepath.empty())
				Instance.reset();

			if (Instance)
			{
				Instance->SetEntity(entity);
				Filepath = filepath;

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
		Ref<Kablunk::Mesh> Mesh;

		MeshComponent(const Ref<Kablunk::Mesh>& mesh)
			: Mesh{ mesh } { }
		MeshComponent(const MeshComponent&) = default;
	};
}

#endif
