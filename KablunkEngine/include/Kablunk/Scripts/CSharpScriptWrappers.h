#ifndef KABLUNK_SCRIPTS_CSHARP_SCRIPT_WRAPPERS_H
#define KABLUNK_SCRIPTS_CSHARP_SCRIPT_WRAPPERS_H

#include "Kablunk/Core/Input.h"
#include "Kablunk/Core/KeyCodes.h"
#include "Kablunk/Core/MouseCodes.h"

#include "Kablunk/Renderer/Texture.h"
#include "Kablunk/Scene/Entity.h"
#include "Kablunk/Scene/Components.h"

#include <glm/glm.hpp>

extern "C"
{
	typedef struct _MonoObject MonoObject;
	typedef struct _MonoString MonoString;
	typedef struct _MonoArray MonoArray;
}

namespace Kablunk::Scripts
{
	// input
	bool			Kablunk_Input_IsKeyPressed(KeyCode key);
	bool			Kablunk_Input_IsMouseButtonPressed(MouseCode button);
	void			Kablunk_Input_GetMousePosition(glm::vec2* out);
	
	// Scene
	MonoArray*		Kablunk_Scene_GetEntities();

	// Entity
	EntityHandle	Kablunk_Entity_GetParent(EntityHandle entity_id);
	void			Kablunk_Entity_SetParent(EntityHandle entity_id, EntityHandle parent_id);
	MonoArray*		Kablunk_Entity_GetChildren(EntityHandle entity_id);
	EntityHandle	Kablunk_Entity_CreateEntity();
	EntityHandle	Kablunk_Entity_DestroyEntity(EntityHandle entity_id);
	void			Kablunk_Entity_CreateComponent(EntityHandle entity_id, void* type);
	bool			Kablunk_Entity_HasComponent(EntityHandle entity_id, void* type);

	// Tag Component
	MonoString*		Kablunk_TagComponent_GetTag(EntityHandle entity_id);
	void			Kablunk_TagComponent_SetTag(EntityHandle entity_id, MonoString* new_tag);

	// Transform Component
	void			Kablunk_TransformComponent_GetTransform(EntityHandle entity_id, TransformComponent* out_transform);
	void			Kablunk_TransformComponent_SetTransform(EntityHandle entity_id, TransformComponent* in_transform);
	void			Kablunk_TransformComponent_GetTranslation(EntityHandle entity_id, glm::vec3* out_translation);
	void			Kablunk_TransformComponent_SetTranslation(EntityHandle entity_id, glm::vec3* in_translation);
	void			Kablunk_TransformComponent_GetRotation(EntityHandle entity_id, glm::vec3* out_rotation);
	void			Kablunk_TransformComponent_SetRotation(EntityHandle entity_id, glm::vec3* in_rotation);
	void			Kablunk_TransformComponent_GetScale(EntityHandle entity_id, glm::vec3* out_scale);
	void			Kablunk_TransformComponent_SetScale(EntityHandle entity_id, glm::vec3* in_scale);

	// Texture2D
	void*			Kablunk_Texture2D_Constructor(uint32_t width, uint32_t height);
	void			Kablunk_Texture2D_Destructor(Ref<Texture2D>* _this);
	void			Kablunk_Texture2D_SetData(Ref<Texture2D>* _this, MonoArray* in_data, int32_t count);

	enum class LogLevel : int32_t
	{
		Trace	= BIT(0),
		Debug	= BIT(1),
		Info	= BIT(2),
		Warn	= BIT(3),
		Error	= BIT(4),
		Critcal = BIT(5),
	};

	void			Kablunk_Log_LogMessage(LogLevel level, MonoString* msg);
}

#endif
