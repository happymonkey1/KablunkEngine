#include "kablunkpch.h"
#include "Kablunk/Scripts/CSharpInternalCallRegistry.h"

#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Kablunk/Scene/Entity.h"
#include "Kablunk/Scripts/CSharpScriptWrappers.h"

namespace kb
{
	std::unordered_map<MonoType*, std::function<bool(Entity&)>> s_has_component_funcs;
	std::unordered_map<MonoType*, std::function<void(Entity&)>> s_create_component_funcs;

	extern MonoImage* s_core_assembly_image;
	#define Component_RegisterType(Type) \
		{\
			MonoType* type = mono_reflection_type_from_name("Kablunk." #Type, s_core_assembly_image);\
			if (type) {\
				uint32_t id = mono_type_get_type(type);\
				s_has_component_funcs[type] = [](Entity& entity) { return entity.HasComponent<Type>(); };\
				s_create_component_funcs[type] = [](Entity& entity) { entity.AddComponent<Type>(); };\
			} else {\
				KB_CORE_ERROR("No C# component class found for " #Type "!");\
			}\
		}

	static void InitComponentTypes()
	{
		Component_RegisterType(TagComponent);
		Component_RegisterType(TransformComponent);
		Component_RegisterType(MeshComponent);
		Component_RegisterType(CSharpScriptComponent);
		Component_RegisterType(CameraComponent);
		Component_RegisterType(SpriteRendererComponent);
		Component_RegisterType(RigidBody2DComponent);
		Component_RegisterType(BoxCollider2DComponent);
		Component_RegisterType(SpriteRendererComponent);
	}



	void CSharpInternalCallRegistry::RegisterAll()
	{
		InitComponentTypes();

		// input
		mono_add_internal_call("Kablunk.Input::IsKeyPressed_Native", Scripts::Kablunk_Input_IsKeyPressed);
		mono_add_internal_call("Kablunk.Input::IsMouseButtonPressed_Native", Scripts::Kablunk_Input_IsMouseButtonPressed);
		mono_add_internal_call("Kablunk.Input::GetMousePosition_Native", Scripts::Kablunk_Input_GetMousePosition);

		// Scene
		mono_add_internal_call("Kablunk.Scene::GetEntities_Native", Scripts::Kablunk_Scene_GetEntities);

		// Entity
		mono_add_internal_call("Kablunk.Entity::GetParent_Native", Scripts::Kablunk_Entity_GetParent);
		mono_add_internal_call("Kablunk.Entity::SetParent_Native", Scripts::Kablunk_Entity_SetParent);
		mono_add_internal_call("Kablunk.Entity::GetChildren_Native", Scripts::Kablunk_Entity_GetChildren);
		mono_add_internal_call("Kablunk.Entity::CreateEntity_Native", Scripts::Kablunk_Entity_CreateEntity);
		mono_add_internal_call("Kablunk.Entity::DestroyEntity_Native", Scripts::Kablunk_Entity_DestroyEntity);
		mono_add_internal_call("Kablunk.Entity::CreateComponent_Native", Scripts::Kablunk_Entity_CreateComponent);
		mono_add_internal_call("Kablunk.Entity::HasComponent_Native", Scripts::Kablunk_Entity_HasComponent);

		// Tag Component
		mono_add_internal_call("Kablunk.TagComponent::GetTag_Native", Scripts::Kablunk_TagComponent_GetTag);
		mono_add_internal_call("Kablunk.TagComponent::SetTag_Native", Scripts::Kablunk_TagComponent_SetTag);

		// Transform Component
		mono_add_internal_call("Kablunk.TransformComponent::GetTransform_Native", Scripts::Kablunk_TransformComponent_GetTransform);
		mono_add_internal_call("Kablunk.TransformComponent::SetTransform_Native", Scripts::Kablunk_TransformComponent_SetTransform);
		mono_add_internal_call("Kablunk.TransformComponent::GetTranslation_Native", Scripts::Kablunk_TransformComponent_GetTranslation);
		mono_add_internal_call("Kablunk.TransformComponent::SetTranslation_Native", Scripts::Kablunk_TransformComponent_SetTranslation);
		mono_add_internal_call("Kablunk.TransformComponent::GetRotation_Native", Scripts::Kablunk_TransformComponent_GetRotation);
		mono_add_internal_call("Kablunk.TransformComponent::SetRotation_Native", Scripts::Kablunk_TransformComponent_SetRotation);
		mono_add_internal_call("Kablunk.TransformComponent::GetScale_Native", Scripts::Kablunk_TransformComponent_GetScale);
		mono_add_internal_call("Kablunk.TransformComponent::SetScale_Native", Scripts::Kablunk_TransformComponent_SetScale);

		// Camera Component
		mono_add_internal_call("Kablunk.Camera::ScreenToWorldPosition_Native", Scripts::Kablunk_CameraComponent_ScreenToWorldPosition);

		// Texture2D
		mono_add_internal_call("Kablunk.Texture2D::Construct_Native", Scripts::Kablunk_Texture2D_Constructor);
		mono_add_internal_call("Kablunk.Texture2D::Destructor_Native", Scripts::Kablunk_Texture2D_Destructor);
		mono_add_internal_call("Kablunk.Texture2D::SetData_Native", Scripts::Kablunk_Texture2D_SetData);

		mono_add_internal_call("Kablunk.Log::LogMessage_Native", Scripts::Kablunk_Log_LogMessage);
	}
}
