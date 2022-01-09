#include "kablunkpch.h"
#include "Kablunk/Scripts/CSharpScriptEngine.h"
#include "Kablunk/Scripts/CSharpInternalCallRegistry.h"
#include "Kablunk/Scene/Components.h"
#include "Kablunk/Scripts/CSharpScriptWrappers.h"

#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/debug-helpers.h>
#include <mono/metadata/attrdefs.h>
#include <mono/metadata/mono-gc.h>

#include <imgui.h>

#include <iostream>
#include <chrono>
#include <thread>

#include <windows.h>
#include <fileapi.h>

#include <box2d/b2_shape.h>
#include <box2d/b2_body.h>
#include <box2d/b2_polygon_shape.h>
#include <box2d/b2_circle_shape.h>
#include <box2d/b2_fixture.h>

// https://www.mono-project.com/docs/advanced/embedding/

namespace Kablunk
{
	namespace Internal
	{
		// #TODO think about moving elsewhere
		bool CheckOnMouseDown(Entity entity, std::pair<float, float> point)
		{
			bool res = false;
			
			//#TODO figure out better way than using the exposed c# api
			glm::vec3 screen_pos{ 0.0f };
			Kablunk::Scripts::Kablunk_CameraComponent_ScreenToWorldPosition(&glm::vec2{ point.first, point.second }, &screen_pos);

			b2Vec2 point_b2D{ screen_pos.x, screen_pos.y };
			

			if (entity.HasComponent<BoxCollider2DComponent>())
			{
				auto& box_collider_comp = entity.GetComponent<BoxCollider2DComponent>();
				b2Fixture* fixture = (b2Fixture*)(box_collider_comp.Runtime_ficture);

				b2Body* body = (b2Body*)(fixture->GetBody());
				const b2Transform& transform = body->GetTransform();

				res = fixture->GetShape()->TestPoint(transform, point_b2D);
				if (res)
					return true;
			}

			if (entity.HasComponent<CircleCollider2DComponent>())
			{
				auto& sphere_collider_comp = entity.GetComponent<CircleCollider2DComponent>();
				b2Fixture* fixture = (b2Fixture*)(sphere_collider_comp.Runtime_ficture);

				b2Body* body = (b2Body*)(fixture->GetBody());
				const b2Transform& transform = body->GetTransform();

				res = fixture->GetShape()->TestPoint(transform, point_b2D) ? true : res;
			}

			return res;
		}

		bool CheckOnMouseOver(Entity entity, std::pair<float, float> point)
		{
#pragma message("CheckOnMouseOver not implemented!")
			return false;
		}

		bool CheckOnMouseMove(Entity entity, std::pair<float, float> point)
		{
#pragma warning("CheckOnMouseMove not implemented!")
			return false;
		}

		bool CheckOnMouseUp(Entity entity, std::pair<float, float> point)
		{
#pragma warning("CheckOnMouseUp not implemented!")
			return false;
		}
	}

	static MonoDomain* s_current_mono_domain = nullptr;
	static MonoDomain* s_new_mono_domain = nullptr;
	static std::string s_core_assembly_path;
	static WeakRef<Scene> s_scene_context = nullptr;

	static EntityInstanceMap s_entity_instance_map;

	static MonoMethod* GetMethod(MonoImage* image, const std::string& method_description);

	MonoImage* s_app_assembly_image = nullptr;
	MonoImage* s_core_assembly_image = nullptr;

	static MonoMethod* s_exception_method = nullptr;
	static MonoClass* s_entity_class = nullptr;

	static std::unordered_map<std::string, MonoClass*> s_classes;
	static std::unordered_map<std::string, EntityScriptClass> s_entity_class_map;
	
	struct EntityScriptClass
	{
		std::string Full_name;
		std::string Class_name;
		std::string Namespace_name;

		MonoClass*  Class				= nullptr;
		MonoMethod* Constructor			= nullptr;
		MonoMethod* OnCreateMethod		= nullptr;
		MonoMethod* OnDestroyMethod		= nullptr;
		MonoMethod* OnUpdateMethod		= nullptr;
		MonoMethod* OnFixedUpdateMethod = nullptr;

		// Physics 2D
		MonoMethod* OnMouseDownMethod	= nullptr;
		MonoMethod* OnMouseOverMethod	= nullptr;
		MonoMethod* OnMouseMoveMethod	= nullptr;
		MonoMethod* OnMouseUpMethod		= nullptr;

		
		void InitClassMethods(MonoImage* image)
		{
			Constructor			= GetMethod(s_core_assembly_image, "Kablunk.Entity:.ctor(ulong)");
			OnCreateMethod		= GetMethod(image, Full_name + ":OnCreate()");
			OnUpdateMethod		= GetMethod(image, Full_name + ":OnUpdate(single)");
			OnFixedUpdateMethod = GetMethod(image, Full_name + ":OnFixedUpdate(single)");
			OnDestroyMethod		= GetMethod(image, Full_name + ":OnDestroy()");

			// Physics 2D
			OnMouseDownMethod	= GetMethod(s_core_assembly_image, "Kablunk.Entity:OnMouseDown()");
			OnMouseOverMethod	= GetMethod(s_core_assembly_image, "Kablunk.Entity:OnMouseOver()");
			OnMouseMoveMethod	= GetMethod(s_core_assembly_image, "Kablunk.Entity:OnMouseMove()");
			OnMouseUpMethod		= GetMethod(s_core_assembly_image, "Kablunk.Entity:OnMouseUp()");
		}
	};

	MonoObject* EntityInstance::GetInstance()
	{
		KB_CORE_ASSERT(Handle, "Entity has not been instantiated!");
		return mono_gchandle_get_target(Handle);
	}

	bool EntityInstance::IsRuntimeAvailable() const
	{
		return Handle != 0;
	}

	MonoAssembly* LoadAssemblyFromFile(const char* filepath)
	{
		if (filepath == nullptr)
			return nullptr;

		HANDLE handle = CreateFileA(filepath, GENERIC_READ, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
		if (handle == INVALID_HANDLE_VALUE)
			return nullptr;

		DWORD file_size = GetFileSize(handle, nullptr);
		if (file_size == INVALID_FILE_SIZE)
		{
			CloseHandle(handle);
			return nullptr;
		}

		void* file_data = malloc(file_size);
		if (file_data == nullptr)
		{
			CloseHandle(handle);
			return nullptr;
		}

		DWORD bytes_read = 0;
		bool read = ReadFile(handle, file_data, file_size, &bytes_read, nullptr);
		if (file_size != bytes_read || !read)
		{
			free(file_data);
			CloseHandle(handle);
			return nullptr;
		}

		MonoImageOpenStatus status = MonoImageOpenStatus::MONO_IMAGE_IMAGE_INVALID;
		MonoImage* image = mono_image_open_from_data_full(reinterpret_cast<char*>(file_data), file_size, 1, &status, 0);
		if (status != MONO_IMAGE_OK)
		{
			free(file_data);
			CloseHandle(handle);
			return nullptr;
		}

		auto assembly = mono_assembly_load_from_full(image, filepath, &status, 0);
		free(file_data);
		CloseHandle(handle);
		mono_image_close(image);
		return assembly;
	}
	
	static void InitMono()
	{
		KB_CORE_TRACE("initializing mono!");
		KB_CORE_ASSERT(!s_current_mono_domain, "[C#-ScriptEngine] Mono has already been initialized!");
		mono_set_assemblies_path("mono/lib");
		auto domain = mono_jit_init("Kablunk");
	}

	static void ShutdownMono()
	{
		//mono_jit_cleanup(s_current_mono_domain);
	}

	static MonoAssembly* LoadAssembly(const std::filesystem::path& path)
	{
		std::string path_as_str = path.string();
		MonoAssembly* assembly = LoadAssemblyFromFile(path_as_str.c_str());
		// can't use loggers here because this happens before they get initialized
		if (!assembly)
		{
			std::cout << "[C#-ScriptEngine] Could not load assembly: " << path_as_str << std::endl;
			return nullptr;
		}
		else
			std::cout << "[C#-ScriptEngine] Successfully loaded assembly: " << path_as_str << std::endl;

		return assembly;
	}

	static MonoImage* GetAssemblyImage(MonoAssembly* assembly)
	{
		MonoImage* image = mono_assembly_get_image(assembly);
		if (!image)
		{
			std::cout << "[C#-ScriptEngine] mono_assembly_get_image failed" << std::endl;
			return nullptr;
		}

		return image;
	}

	static MonoClass* GetClass(MonoImage* image, const EntityScriptClass& script_class)
	{
		MonoClass* mono_class = mono_class_from_name(image, script_class.Namespace_name.c_str(), script_class.Class_name.c_str());
		if (!mono_class)
		{
			std::cout << "[C#-ScriptEngine] mono_class_from_name failed: " << script_class.Namespace_name << "::" << script_class.Full_name << std::endl;
			return nullptr;
		}

		return mono_class;
	}

	static uint32_t Instantiate(EntityScriptClass& script_class)
	{
		KB_PROFILE_FUNCTION();

		// allocate memory
		MonoObject* instance = mono_object_new(s_current_mono_domain, script_class.Class);
		if (!instance)
			std::cout << "[C#-ScriptEngine] mono_object_new failed: " << script_class.Class << std::endl;

		mono_runtime_object_init(instance);
		uint32_t handle = mono_gchandle_new(instance, false);
		KB_CORE_TRACE("[C#-ScriptEngine] instantiated script '{0}'", script_class.Full_name);

		return handle;
	}

	static void Destroy(uint32_t handle)
	{
		mono_gchandle_free(handle);
	}

	static MonoMethod* GetMethod(MonoImage* image, const std::string& method_description)
	{
		KB_CORE_ASSERT(image, "MonoImage* is invalid!");

		MonoMethodDesc* description = mono_method_desc_new(method_description.c_str(), false);
		if (!description)
		{
			KB_CORE_ERROR("[C#-ScriptEngine] mono_method_desc_new failed ({0})", method_description);
			return nullptr;
		}

		MonoMethod* method = mono_method_desc_search_in_image(description, image);
		if (!method)
		{
			KB_CORE_WARN("[C#-ScriptEngine] mono_method_desc_search_in_image failed ({0})", method_description);
			return nullptr;
		}

		return method;
	}

	static std::string GetStringProperty(const char* property_name, MonoClass* klass, MonoObject* object)
	{
		MonoProperty* prop = mono_class_get_property_from_name(klass, property_name);
		MonoMethod* getter_method = mono_property_get_get_method(prop);
		MonoString* string = (MonoString*)mono_runtime_invoke(getter_method, object, nullptr, nullptr);
		return string != nullptr ? std::string{ mono_string_to_utf8(string) } : "";
	}

	static MonoObject* CallMethod(MonoObject* object, MonoMethod* method, void** params = nullptr)
	{
		MonoObject* except = nullptr;
		MonoObject* result = mono_runtime_invoke(method, object, params, &except);
		if (except)
		{
			MonoClass* exception_class = mono_object_get_class(except);
			MonoType* exception_type = mono_class_get_type(exception_class);
			const char* type_name = mono_type_get_name(exception_type);
			std::string message = GetStringProperty("Message", exception_class, except);
			std::string stack_trace = GetStringProperty("StackTrace", exception_class, except);

			KB_CORE_ERROR("[C#-ScriptEngine] {0} : {1}. Stack Trace: {2}", type_name, message, stack_trace);

			void* args[] = { except };
			result = mono_runtime_invoke(s_exception_method, nullptr, args, nullptr);
		}

		return result;
	}

	static void PrintclassMethods(MonoClass* mono_class)
	{
		KB_CORE_ASSERT(false, "not implemented!");
	}

	static MonoString* GetName()
	{
		KB_CORE_WARN("GetName() not implemented!");
		return mono_string_new(s_current_mono_domain, "Foobar");
	}

	static MonoAssembly* s_app_assembly = nullptr;
	static MonoAssembly* s_core_assembly = nullptr;
	static bool s_post_load_cleanup = false;

	void CSharpScriptEngine::Init(const std::filesystem::path& assembly_path)
	{
		InitMono();
		LoadKablunkRuntimeAssembly(assembly_path);
	}

	void CSharpScriptEngine::Shutdown()
	{
		ShutdownMono();
		s_scene_context = nullptr;
	}

	void CSharpScriptEngine::OnSceneDestroy(uuid::uuid64 scene_id)
	{
		if (s_entity_instance_map.find(scene_id) != s_entity_instance_map.end())
		{
			s_entity_instance_map.at(scene_id).clear();
			s_entity_instance_map.erase(scene_id);
		}
	}

	bool CSharpScriptEngine::LoadKablunkRuntimeAssembly(const std::filesystem::path& path)
	{
		s_core_assembly_path = path.string();
		if (s_current_mono_domain)
		{
			s_new_mono_domain = mono_domain_create_appdomain("Kablunk Runtime", nullptr);
			mono_domain_set(s_new_mono_domain, false);
			s_post_load_cleanup = true;
		}
		else
		{
			s_current_mono_domain = mono_domain_create_appdomain("Kablunk Runtime", nullptr);
			mono_domain_set(s_current_mono_domain, false);
			s_post_load_cleanup = false;
		}

		s_core_assembly = LoadAssembly(s_core_assembly_path);
		if (!s_core_assembly)
			return false;

		s_core_assembly_image = GetAssemblyImage(s_core_assembly);

		s_exception_method = GetMethod(s_core_assembly_image, "Kablunk.RuntimeException:OnException(object)");
		s_entity_class = mono_class_from_name(s_core_assembly_image, "Kablunk", "Entity");

		return true;
	}

	bool CSharpScriptEngine::LoadAppAssembly(const std::filesystem::path& path)
	{
		if (s_app_assembly)
		{
			s_app_assembly = nullptr;
			s_app_assembly_image = nullptr;
			return ReloadAssembly(path);
		}

		auto app_assembly = LoadAssembly(path);
		if (!app_assembly)
			return false;

		auto app_assembly_image = GetAssemblyImage(app_assembly);
		CSharpInternalCallRegistry::RegisterAll();
		
		if (s_post_load_cleanup)
		{
			mono_domain_unload(s_current_mono_domain);
			s_current_mono_domain = s_new_mono_domain;
			s_new_mono_domain = nullptr;
		}

		s_app_assembly = app_assembly;
		s_app_assembly_image = app_assembly_image;
		return true;
	}

	bool CSharpScriptEngine::ReloadAssembly(const std::filesystem::path& path)
	{
		KB_CORE_INFO("Reloading C# assemblies");
		if (!LoadKablunkRuntimeAssembly(s_core_assembly_path))
			return false;

		if (!LoadAppAssembly(path))
			return false;

		if (!s_entity_instance_map.empty())
		{
			WeakRef<Scene> scene = CSharpScriptEngine::GetCurrentSceneContext();
			KB_CORE_ASSERT(scene, "[C#-ScriptEngine] No active scene");
			if (auto& entity_instance_map = s_entity_instance_map.find(scene->GetUUID()); entity_instance_map != s_entity_instance_map.end())
			{
				const auto& entity_map = scene->GetEntityMap();
				for (auto& [entity_id, entity_instance_data] : entity_instance_map->second)
				{
					KB_CORE_ASSERT(entity_map.find(entity_id) != entity_map.end(), "Invalid entity id '{0}'", entity_id);
					InitScriptEntity(entity_map.at(entity_id));
				}
			}
		}
	}

	void CSharpScriptEngine::SetSceneContext(Scene* scene)
	{
		s_classes.clear();
		if (!scene)
			s_entity_instance_map.clear();
		s_scene_context = scene;
	}

	const WeakRef<Scene>& CSharpScriptEngine::GetCurrentSceneContext()
	{
		return s_scene_context;
	}

	void CSharpScriptEngine::CopyEntityScriptData(uuid::uuid64 dst, uuid::uuid64 src)
	{
		KB_CORE_ASSERT(s_entity_instance_map.find(src) != s_entity_instance_map.end(), "src entity not found in map!");
		KB_CORE_ASSERT(s_entity_instance_map.find(dst) != s_entity_instance_map.end(), "src entity not found in map!");

		auto dst_entity_map = s_entity_instance_map.at(dst);
		auto src_entity_map = s_entity_instance_map.at(src);

		//KB_CORE_ASSERT(false, "not implemented!");
		// #TODO implement copy of public fields
	}

	void CSharpScriptEngine::OnCreateEntity(Entity entity)
	{
		auto& entity_instance = GetEntityInstanceData(entity.GetSceneUUID(), entity.GetUUID()).Instance;
		if (entity_instance.Script_class->OnCreateMethod)
			CallMethod(entity_instance.GetInstance(), entity_instance.Script_class->OnCreateMethod);
	}

	void CSharpScriptEngine::OnUpdateEntity(Entity entity, Timestep ts)
	{
		auto& entity_instance = GetEntityInstanceData(entity.GetSceneUUID(), entity.GetUUID()).Instance;
		if (entity_instance.Script_class->OnUpdateMethod)
		{
			void* args[] = { &ts };
			CallMethod(entity_instance.GetInstance(), entity_instance.Script_class->OnUpdateMethod, args);
		}
	}

	void CSharpScriptEngine::OnFixedUpdateEntity(Entity entity, Timestep fixed_ts)
	{
		auto& entity_instance = GetEntityInstanceData(entity.GetSceneUUID(), entity.GetUUID()).Instance;
		if (entity_instance.Script_class->OnFixedUpdateMethod)
		{
			void* args[] = { &fixed_ts };
			CallMethod(entity_instance.GetInstance(), entity_instance.Script_class->OnFixedUpdateMethod, args);
		}
	}

	void CSharpScriptEngine::OnMouseDownEntity(Entity entity)
	{
		auto& entity_instance = GetEntityInstanceData(entity.GetSceneUUID(), entity.GetUUID()).Instance;
		if (entity_instance.Script_class->OnMouseDownMethod)
			if (Internal::CheckOnMouseDown(entity, Input::GetMousePosition()))
				CallMethod(entity_instance.GetInstance(), entity_instance.Script_class->OnMouseDownMethod, nullptr);
	}

	void CSharpScriptEngine::OnMouseOverEntity(Entity entity)
	{
		auto& entity_instance = GetEntityInstanceData(entity.GetSceneUUID(), entity.GetUUID()).Instance;
		if (entity_instance.Script_class->OnMouseDownMethod)
			if (Internal::CheckOnMouseOver(entity, Input::GetMousePosition()))
				CallMethod(entity_instance.GetInstance(), entity_instance.Script_class->OnMouseOverMethod, nullptr);
	}

	void CSharpScriptEngine::OnMouseMoveEntity(Entity entity)
	{
		auto& entity_instance = GetEntityInstanceData(entity.GetSceneUUID(), entity.GetUUID()).Instance;
		if (entity_instance.Script_class->OnMouseDownMethod)
			if (Internal::CheckOnMouseMove(entity, Input::GetMousePosition()))
				CallMethod(entity_instance.GetInstance(), entity_instance.Script_class->OnMouseMoveMethod, nullptr);
	}

	void CSharpScriptEngine::OnMouseUpEntity(Entity entity)
	{
		auto& entity_instance = GetEntityInstanceData(entity.GetSceneUUID(), entity.GetUUID()).Instance;
		if (entity_instance.Script_class->OnMouseDownMethod)
			if (Internal::CheckOnMouseUp(entity, Input::GetMousePosition()))
				CallMethod(entity_instance.GetInstance(), entity_instance.Script_class->OnMouseUpMethod, nullptr);
	}

	void CSharpScriptEngine::OnScriptComponentDestroyed(uuid::uuid64 scene_id, uuid::uuid64 entity_id)
	{
		if (s_entity_instance_map.find(scene_id) != s_entity_instance_map.end())
		{
			auto& entity_map = s_entity_instance_map.at(scene_id);
			if (entity_map.find(entity_id) != entity_map.end())
				entity_map.erase(entity_id);
		}
	}

	MonoObject* CSharpScriptEngine::Construct(const std::string& full_name, bool call_constructor /*= true*/, void** parameters /*= nullptr*/)
	{
		std::string namespace_name;
		std::string class_name;
		std::string parameter_list;

		if (full_name.find('.') != std::string::npos)
		{
			namespace_name = full_name.substr(0, full_name.find_first_of('.'));
			class_name = full_name.substr(full_name.find_first_of('.') + 1, (full_name.find_first_of(':') - full_name.find_first_of('.') - 1));
		}

		if (full_name.find(':') != std::string::npos)
		{
			parameter_list = full_name.substr(full_name.find_first_of(':'));
		}

		MonoClass* klass = mono_class_from_name(s_core_assembly_image, namespace_name.c_str(), class_name.c_str());
		KB_CORE_ASSERT(klass, "[C#-ScriptCore] could not find class '{0}:{1}'", namespace_name, class_name);
		MonoObject* obj = mono_object_new(mono_domain_get(), klass);

		if (call_constructor)
		{
			MonoMethodDesc* description = mono_method_desc_new(parameter_list.c_str(), false);
			MonoMethod* ctor = mono_method_desc_search_in_class(description, klass);
			MonoObject* except = nullptr;
			mono_runtime_invoke(ctor, obj, parameters, &except);
		}

		return obj;
	}

	MonoClass* CSharpScriptEngine::GetCoreClass(const std::string& full_name)
	{
		KB_CORE_ASSERT(false, "not implemented!");
		return nullptr;
	}

	bool CSharpScriptEngine::ModuleExists(const std::string& module_name)
	{
		// no assembly loaded
		if (!s_app_assembly_image)
			return false;

		std::string namespace_name, class_name;
		if (module_name.find('.') != std::string::npos)
		{
			size_t p = module_name.find_last_of('.');
			namespace_name = module_name.substr(0, p);
			class_name = module_name.substr(p + 1);
		}
		else
			class_name = module_name;

		MonoClass* mono_class = mono_class_from_name(s_app_assembly_image, namespace_name.c_str(), class_name.c_str());
		if (!mono_class)
			return false;

		auto is_entity_sub_class = mono_class_is_subclass_of(mono_class, s_entity_class, false);
		return is_entity_sub_class;
	}

	std::string CSharpScriptEngine::StripNamespace(const std::string& name_space, const std::string& module_name)
	{
		if (module_name.empty())
		{
			KB_CORE_ERROR("module name empty");
			return std::string{ "" };
		}

		std::string name = module_name;
		size_t p = name.find(name_space + ".");
		if (p == 0)
			name.erase(p, name_space.size() + 1);

		return name;
	}

	void CSharpScriptEngine::InitScriptEntity(Entity entity)
	{
		Scene* context = entity.m_scene;
		uuid::uuid64 id = entity.GetComponent<IdComponent>().Id;
		auto& comp = entity.GetComponent<CSharpScriptComponent>();
		auto& module_name = comp.Module_name;
		if (module_name.empty())
			return;

		if (!ModuleExists(module_name))
		{
			KB_CORE_ERROR("Entity references non-existant script module '{0}'", module_name);
			return;
		}

		EntityScriptClass& script_class = s_entity_class_map[module_name];
		script_class.Full_name = module_name;
		if (module_name.find('.') != std::string::npos)
		{
			size_t p = module_name.find_last_of('.');
			script_class.Namespace_name = module_name.substr(0, p);
			script_class.Class_name = module_name.substr(p + 1);
		}
		else
			script_class.Class_name = module_name;

		script_class.Class = GetClass(s_app_assembly_image, script_class);
		script_class.InitClassMethods(s_app_assembly_image);

		KB_CORE_INFO("InitScriptEntity called for scene '{0}'", context->GetUUID());
		
		EntityInstanceData& entity_instance_data = s_entity_instance_map[context->GetUUID()][id];
		EntityInstance& entity_instance = entity_instance_data.Instance;
		entity_instance.Script_class = &script_class;

		CSharpScriptComponent& script_comp = entity.GetComponent<CSharpScriptComponent>();
		// #TODO save old fields so reloading scripts maintains values

		// #TODO retrieve public fields to display in gui

		Destroy(entity_instance.Handle);
	}

	void CSharpScriptEngine::ShutdownScriptEntity(Entity entity, const std::string& module_name)
	{
		KB_CORE_ERROR("ShutdownScriptEntity not implemented!");
	}

	void CSharpScriptEngine::InstantiateEntityClass(Entity entity)
	{
		Scene* context = entity.m_scene;
		UUID id = entity.GetComponent<IdComponent>().Id;
		KB_CORE_TRACE("InstantiateEntityClass {0} ({1})", id, entity.m_entity_handle);
		auto& script_comp = entity.GetComponent<CSharpScriptComponent>();
		auto& module_name = script_comp.Module_name;

		EntityInstanceData& entity_instance_data = GetEntityInstanceData(context->GetUUID(), id);
		EntityInstance& entity_instance = entity_instance_data.Instance;
		KB_CORE_ASSERT(entity_instance.Script_class, "Script class not set for instance!");
		entity_instance.Handle = Instantiate(*entity_instance.Script_class);

		void* args[] = { &id };
		CallMethod(entity_instance.GetInstance(), entity_instance.Script_class->Constructor, args);

		// #TODO set public field values
	}

	EntityInstanceMap& CSharpScriptEngine::GetEntityInstanceMap()
	{
		return s_entity_instance_map;
	}

	EntityInstanceData& CSharpScriptEngine::GetEntityInstanceData(uuid::uuid64 scene_id, uuid::uuid64 entity_id)
	{
		KB_CORE_ASSERT(s_entity_instance_map.find(scene_id) != s_entity_instance_map.end(), "invalid scene id");
		auto& entity_id_map = s_entity_instance_map.at(scene_id);

		if (entity_id_map.find(entity_id) == entity_id_map.end())
			CSharpScriptEngine::InitScriptEntity(s_scene_context->GetEntityMap().at(entity_id));

		return entity_id_map.at(entity_id);
	}

	void CSharpScriptEngine::OnImGuiRender()
	{
		ImGui::Begin("C# Script Engine Debug");

		float gc_heap_size = (float)mono_gc_get_heap_size();
		float gc_usage_size = (float)mono_gc_get_used_size();
		ImGui::Text("GC Heap Info(Used/Available): %.2fKB/%.2fKB", gc_usage_size / 1024.0f, gc_heap_size / 1024.0f);

		for (auto& [sceneID, entityMap] : s_entity_instance_map)
		{
			bool opened = ImGui::TreeNode((void*)(uint64_t)sceneID, "Scene (%llx)", sceneID);
			if (opened)
			{
				WeakRef<Scene> scene = Scene::GetScene(sceneID);
				for (auto& [entityID, entityInstanceData] : entityMap)
				{
					Entity entity = scene->GetEntityMap().at(entityID);
					std::string entityName = "Unnamed Entity";
					if (entity.HasComponent<TagComponent>())
						entityName = entity.GetComponent<TagComponent>().Tag;
					opened = ImGui::TreeNode((void*)(uint64_t)entityID, "%s (%llx)", entityName.c_str(), entityID);
					if (opened)
					{
						// #TODO public c# script fields
						ImGui::TreePop();
					}
				}
				ImGui::TreePop();
			}
		}

		ImGui::End();
	}
}
