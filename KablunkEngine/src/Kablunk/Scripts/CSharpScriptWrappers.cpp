#include "kablunkpch.h"
#include "Kablunk/Scripts/CSharpScriptWrappers.h"

#include "Kablunk/Scripts/CSharpScriptEngine.h"
#include "Kablunk/Core/Application.h"

#include "Kablunk/Core/MouseCodes.h"

#include <mono/jit/jit.h>

#include <imgui.h>
#include <imgui_internal.h>

#include <box2d/b2_shape.h>
#include <box2d/b2_body.h>
#include <box2d/b2_polygon_shape.h>
#include <box2d/b2_circle_shape.h>
#include <box2d/b2_fixture.h>

namespace kb {
	extern std::unordered_map<MonoType*, std::function<bool(Entity&)>> s_has_component_funcs;
	extern std::unordered_map<MonoType*, std::function<void(Entity&)>> s_create_component_funcs;
}


namespace kb::Scripts
{

	static inline auto GetEntity(uint64_t entity_id)
	{
		WeakRef<Scene> context = CSharpScriptEngine::GetCurrentSceneContext();
		KB_CORE_ASSERT(context, "No active scene!");

		const auto& entity_map = context->GetEntityMap();
		KB_CORE_ASSERT(entity_map.find(entity_id) != entity_map.end(), "Entity does not exist in current context!");

		return entity_map.at(entity_id);
	}

	bool Kablunk_Input_IsKeyPressed(KeyCode key)
	{
		return input::is_key_pressed(key);
	}

	bool Kablunk_Input_IsMouseButtonPressed(MouseCode button)
	{
		bool pressed = input::is_mouse_button_pressed(button);

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
		auto [x, y] = input::get_mouse_position();
		*out = { x, y };
	}

	MonoArray* Kablunk_Scene_GetEntities()
	{
		kb::WeakRef<Scene> context = CSharpScriptEngine::GetCurrentSceneContext();
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

	uint64_t Kablunk_Entity_GetParent(uint64_t entity_id)
	{
		KB_CORE_ASSERT(false, "not implemented!");
		return 0;
	}

	void Kablunk_Entity_SetParent(uint64_t entity_id, uint64_t parent_id)
	{
		KB_CORE_ASSERT(false, "not implemented!");
	}

	MonoArray* Kablunk_Entity_GetChildren(uint64_t entity_id)
	{
		KB_CORE_ASSERT(false, "not implemented!");
		return nullptr;
	}

	uint64_t Kablunk_Entity_CreateEntity()
	{
		WeakRef<Scene> context = CSharpScriptEngine::GetCurrentSceneContext();
		return context->CreateEntity("New C# Entity").GetUUID();
	}

	uint64_t Kablunk_Entity_DestroyEntity(uint64_t entity_id)
	{
		KB_CORE_ASSERT(false, "not implemented!");
		return 0;
	}

	void Kablunk_Entity_CreateComponent(uint64_t entity_id, void* type)
	{
		auto entity = GetEntity(entity_id);
		MonoType* monoType = mono_reflection_type_get_type((MonoReflectionType*)type);
		s_create_component_funcs[monoType](entity);
	}

	bool Kablunk_Entity_HasComponent(uint64_t entity_id, void* type)
	{
		auto entity = GetEntity(static_cast<uint64_t>(entity_id));
		MonoType* mono_type = mono_reflection_type_get_type((MonoReflectionType*)type);
		bool res = s_has_component_funcs[mono_type](entity);
		return res;
	}

	MonoString* Kablunk_TagComponent_GetTag(uint64_t entity_id)
	{
		auto entity = GetEntity(entity_id);
		return mono_string_new(mono_domain_get(), entity.GetComponent<TagComponent>().Tag.c_str());
	}

	void Kablunk_TagComponent_SetTag(uint64_t entity_id, MonoString* new_tag)
	{
		auto entity = GetEntity(entity_id);
		entity.GetComponent<TagComponent>().Tag = mono_string_to_utf8(new_tag);
	}

	void Kablunk_TransformComponent_GetTransform(uint64_t entity_id, TransformComponent* out_transform)
	{
		auto entity = GetEntity(entity_id);
		*out_transform = entity.GetComponent<TransformComponent>();
	}

	void Kablunk_TransformComponent_SetTransform(uint64_t entity_id, TransformComponent* in_transform)
	{
		auto entity = GetEntity(entity_id);
		entity.GetComponent<TransformComponent>() = *in_transform;

		if (entity.HasComponent<RigidBody2DComponent>())
		{
			auto& rb2d_comp = entity.GetComponent<RigidBody2DComponent>();
			b2Body* body = (b2Body*)rb2d_comp.Runtime_body;
			body->SetTransform(b2Vec2{ in_transform->Translation.x, in_transform->Translation.y }, in_transform->Rotation.z);
		}
	}

	void Kablunk_TransformComponent_GetTranslation(uint64_t entity_id, glm::vec3* out_translation)
	{
		auto entity = GetEntity(entity_id);
		*out_translation = entity.GetComponent<TransformComponent>().Translation;
	}

	void Kablunk_TransformComponent_SetTranslation(uint64_t entity_id, glm::vec3* in_translation)
	{
		auto entity = GetEntity(entity_id);
		entity.GetComponent<TransformComponent>().Translation = *in_translation;

		if (entity.HasComponent<RigidBody2DComponent>())
		{
			auto& rb2d_comp = entity.GetComponent<RigidBody2DComponent>();
			b2Body* body = (b2Body*)rb2d_comp.Runtime_body;
			body->SetTransform(b2Vec2{ in_translation->x, in_translation->y }, body->GetAngle());
		}
	}

	void Kablunk_TransformComponent_GetRotation(uint64_t entity_id, glm::vec3* out_rotation)
	{
		auto entity = GetEntity(entity_id);
		*out_rotation = entity.GetComponent<TransformComponent>().Rotation;
	}

	void Kablunk_TransformComponent_SetRotation(uint64_t entity_id, glm::vec3* in_rotation)
	{
		auto entity = GetEntity(entity_id);
		entity.GetComponent<TransformComponent>().Rotation = *in_rotation;

		if (entity.HasComponent<RigidBody2DComponent>())
		{
			auto& rb2d_comp = entity.GetComponent<RigidBody2DComponent>();
			b2Body* body = (b2Body*)rb2d_comp.Runtime_body;
			const b2Transform& transform = body->GetTransform();
			body->SetTransform(body->GetPosition(), in_rotation->z);
		}
	}

	void Kablunk_TransformComponent_GetScale(uint64_t entity_id, glm::vec3* out_scale)
	{
		auto entity = GetEntity(entity_id);
		*out_scale = entity.GetComponent<TransformComponent>().Scale;
	}

	void Kablunk_TransformComponent_SetScale(uint64_t entity_id, glm::vec3* in_scale)
	{
		auto entity = GetEntity(entity_id);
		entity.GetComponent<TransformComponent>().Scale = *in_scale;
	}

	void Kablunk_CameraComponent_ScreenToWorldPosition(glm::vec2* screen_pos, glm::vec3* out_position)
	{
		WeakRef<Scene> context = CSharpScriptEngine::GetCurrentSceneContext();
		KB_CORE_ASSERT(context, "no context set!");

		auto entity = context->GetPrimaryCameraEntity();
		if (!entity.Valid())
		{
			KB_CORE_ERROR("no valid primary camera entity!");
			return;
		}

		auto& camera = entity.GetComponent<CameraComponent>().Camera;
		auto transform = entity.GetComponent<TransformComponent>().GetTransform();
		glm::mat4 mat_projection = glm::inverse(transform) * camera.GetProjection();
		glm::mat4 inverse = glm::inverse(mat_projection);
		
		glm::vec2 window_size;
		
		if (Application::Get().GetSpecification().Enable_imgui)
		{
			ImGuiContext* imgui_context = ImGui::GetCurrentContext();
			ImGuiWindow* imgui_window = imgui_context->HoveredWindow;
			if (imgui_window)
				window_size = glm::vec2{ imgui_window->Size.x, imgui_window->Size.y };
			else
				return;
		}
		else
			window_size = Application::Get().GetWindowDimensions();
		
		glm::vec2 transformed_pos = {
			(2.0f * (screen_pos->x / window_size.x)) - 1.0f,
			1.0f - (2.0f * (screen_pos->y / window_size.y))
		};
		glm::vec4 in = { transformed_pos.x, transformed_pos.y, 0.0f, 1.0f };

		glm::vec4 res_out = in * inverse;

		out_position->x = res_out.x;
		out_position->y = res_out.y;
		out_position->z = res_out.z;

		*out_position /= res_out.w;
	}
	

	void* Kablunk_Texture2D_Constructor(uint32_t width, uint32_t height)
	{
		KB_CORE_ASSERT(false, "not implemented!");
		return nullptr;
	}

	void Kablunk_Texture2D_Destructor(ref<Texture2D>* _this)
	{
		KB_CORE_ASSERT(false, "not implemented!");
	}

	void Kablunk_Texture2D_SetData(ref<Texture2D>* _this, MonoArray* in_data, int32_t count)
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
