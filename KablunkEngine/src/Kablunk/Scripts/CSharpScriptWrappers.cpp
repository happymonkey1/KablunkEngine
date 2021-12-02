#include "kablunkpch.h"
#include "Kablunk/Scripts/CSharpScriptWrappers.h"

#include "Kablunk/Scripts/CSharpScriptEngine.h"
#include "Kablunk/Core/Application.h"

#include <mono/jit/jit.h>

#include <imgui.h>
#include <imgui_internal.h>

namespace Kablunk::Scripts
{

	bool Kablunk_Input_IsKeyPressed(KeyCode key)
	{
		return Input::IsKeyPressed(key);
	}

	bool Kablunk_Input_IsMouseButtonPressed(MouseCode button)
	{
		bool pressed = Input::IsMouseButtonPressed(button);

		// If we are in the editor, we need to make sure mouse pressed only get sent when viewport is hovered
		bool imgui_enabled = Application::Get().GetSpecification().Enable_imgui;
		if (pressed && imgui_enabled && GImGui->HoveredWindow != nullptr)
		{
			ImGuiWindow* viewport_window = ImGui::FindWindowByName("Viewport");
			if (viewport_window != nullptr)
				pressed = GImGui->HoveredWindow->ID == viewport_window->ID;
		}

		return pressed;
	}

	void Kablunk_Input_GetMousePosition(glm::vec2* out)
	{
		auto [x, y] = Input::GetMousePosition();
		*out = { x, y };
	}

	MonoArray* Kablunk_Scene_GetEntities()
	{
		auto context = CSharpScriptEngine::GetCurrentSceneContext();
		KB_CORE_ASSERT(context, "no scene set!");
		const auto& entity_map = context->GetEntityMap();

		MonoArray* result = mono_array_new(mono_domain_get(), CSharpScriptEngine::GetCoreClass("Kablunk.Entity"), entity_map.size());

		uint32_t index = 0;
		for (auto& [id, entity] : entity_map)
		{
			uuid::uuid64 uuid = id;
			void* args[] = { &uuid };
			MonoObject* obj = CSharpScriptEngine::Construct("Kablunk.Entity::ctor(unlong)", true, args);
			mono_array_set(result, MonoObject*, index++, obj);
		}

		return result;
	}

	EntityHandle Kablunk_Entity_GetParent(EntityHandle entity_id)
	{
		KB_CORE_ASSERT(false, "not implemented!");
		return (EntityHandle)0;
	}

	void Kablunk_Entity_SetParent(EntityHandle entity_id, EntityHandle parent_id)
	{
		KB_CORE_ASSERT(false, "not implemented!");
	}

	MonoArray* Kablunk_Entity_GetChildren(EntityHandle entity_id)
	{
		KB_CORE_ASSERT(false, "not implemented!");
		return nullptr;
	}

	EntityHandle Kablunk_Entity_CreateEntity()
	{
		KB_CORE_ASSERT(false, "not implemented!");
		return (EntityHandle)0;
	}

	EntityHandle Kablunk_Entity_DestroyEntity(EntityHandle entity_id)
	{
		KB_CORE_ASSERT(false, "not implemented!");
		return (EntityHandle)0;
	}

	void Kablunk_Entity_CreateComponent(EntityHandle entity_id, void* type)
	{
		KB_CORE_ASSERT(false, "not implemented!");
	}

	bool Kablunk_Entity_HasComponent(EntityHandle entity_id, void* type)
	{
		KB_CORE_ASSERT(false, "not implemented!");
		return false;
	}

	MonoString* Kablunk_TagComponent_GetTag(EntityHandle entity_id)
	{
		KB_CORE_ASSERT(false, "not implemented!");
		return nullptr;
	}

	void Kablunk_TagComponent_SetTag(EntityHandle entity_id, MonoString* new_tag)
	{
		KB_CORE_ASSERT(false, "not implemented!");
	}

	void Kablunk_TransformComponent_GetTransform(EntityHandle entity_id, TransformComponent* out_transform)
	{
		KB_CORE_ASSERT(false, "not implemented!");
	}

	void Kablunk_TransformComponent_SetTransform(EntityHandle entity_id, TransformComponent* in_transform)
	{
		KB_CORE_ASSERT(false, "not implemented!");
	}

	void Kablunk_TransformComponent_GetTranslation(EntityHandle entity_id, glm::vec3* out_translation)
	{
		KB_CORE_ASSERT(false, "not implemented!");
	}

	void Kablunk_TransformComponent_SetTranslation(EntityHandle entity_id, glm::vec3* in_translation)
	{
		KB_CORE_ASSERT(false, "not implemented!");
	}

	void Kablunk_TransformComponent_GetRotation(EntityHandle entity_id, glm::vec3* out_rotation)
	{
		KB_CORE_ASSERT(false, "not implemented!");
	}

	void Kablunk_TransformComponent_SetRotation(EntityHandle entity_id, glm::vec3* in_rotation)
	{
		KB_CORE_ASSERT(false, "not implemented!");
	}

	void Kablunk_TransformComponent_GetScale(EntityHandle entity_id, glm::vec3* out_scale)
	{
		KB_CORE_ASSERT(false, "not implemented!");
	}

	void Kablunk_TransformComponent_SetScale(EntityHandle entity_id, glm::vec3* in_scale)
	{
		KB_CORE_ASSERT(false, "not implemented!");
	}

	void* Kablunk_Texture2D_Constructor(uint32_t width, uint32_t height)
	{
		KB_CORE_ASSERT(false, "not implemented!");
		return nullptr;
	}

	void Kablunk_Texture2D_Destructor(Ref<Texture2D>* _this)
	{
		KB_CORE_ASSERT(false, "not implemented!");
	}

	void Kablunk_Texture2D_SetData(Ref<Texture2D>* _this, MonoArray* in_data, int32_t count)
	{
		KB_CORE_ASSERT(false, "not implemented!");
	}

	void Kablunk_Log_LogMessage(LogLevel level, MonoString* msg)
	{
		// #TODO console in editor
		char* msg_cstr = mono_string_to_utf8(msg);
		switch (level)
		{
			case LogLevel::Info:	KB_CORE_INFO(msg_cstr); break;
			case LogLevel::Debug:	KB_CORE_INFO(msg_cstr); break;
			case LogLevel::Trace:	KB_CORE_TRACE(msg_cstr); break;
			case LogLevel::Warn:	KB_CORE_WARN(msg_cstr); break;
			case LogLevel::Error:	KB_CORE_ERROR(msg_cstr); break;
			case LogLevel::Critcal:	KB_CORE_FATAL(msg_cstr); break;
		}
	}

}
