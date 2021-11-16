#include "kablunkpch.h"
#include "Kablunk/Scripts/CSharpInternalCallRegistry.h"

#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Kablunk/Scene/Entity.h"
#include "Kablunk/Scripts/CSharpScriptWrappers.h"

namespace Kablunk
{

	void CSharpInternalCallRegistry::RegisterAll()
	{
		// input
		mono_add_internal_call("Kablunk.Input::IsKeyPressed_Native", Scripts::Kablunk_Input_IsKeyPressed);
		mono_add_internal_call("Kablunk.Input::IsMouseButtonPressed_Native", Scripts::Kablunk_Input_IsMouseButtonPressed);
		mono_add_internal_call("Kablunk.Input::IsKeyPressed_Native", Scripts::Kablunk_Input_GetMousePosition);

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
		mono_add_internal_call("Kablunk.Transform::GetTransform_Native", Scripts::Kablunk_TransformComponent_GetTransform);
		mono_add_internal_call("Kablunk.Transform::SetTransform_Native", Scripts::Kablunk_TransformComponent_SetTransform);
		mono_add_internal_call("Kablunk.Transform::GetTranslation_Native", Scripts::Kablunk_TransformComponent_GetTranslation);
		mono_add_internal_call("Kablunk.Transform::SetTranslation_Native", Scripts::Kablunk_TransformComponent_SetTranslation);
		mono_add_internal_call("Kablunk.Transform::GetRotation_Native", Scripts::Kablunk_TransformComponent_GetRotation);
		mono_add_internal_call("Kablunk.Transform::SetRotation_Native", Scripts::Kablunk_TransformComponent_SetRotation);
		mono_add_internal_call("Kablunk.Transform::GetScale_Native", Scripts::Kablunk_TransformComponent_GetScale);
		mono_add_internal_call("Kablunk.Transform::SetScale_Native", Scripts::Kablunk_TransformComponent_SetScale);

		// Texture2D
		mono_add_internal_call("Kablunk.Texture2D::Construct_Native", Scripts::Kablunk_Texture2D_Constructor);
		mono_add_internal_call("Kablunk.Texture2D::Destructor_Native", Scripts::Kablunk_Texture2D_Destructor);
		mono_add_internal_call("Kablunk.Texture2D::SetData_Native", Scripts::Kablunk_Texture2D_SetData);
	}
}
