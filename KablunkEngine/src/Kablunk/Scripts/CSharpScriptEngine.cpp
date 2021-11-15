#include "kablunkpch.h"
#include "Kablunk/Scripts/CSharpScriptEngine.h"

#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/debug-helpers.h>
#include <mono/metadata/attrdefs.h>
#include <mono/metadata/mono-gc.h>

#include <iostream>
#include <chrono>
#include <thread>

#include <windows.h>
#include <fileapi.h>

// https://www.mono-project.com/docs/advanced/embedding/

namespace Kablunk
{
	static MonoDomain* s_current_mono_domain = nullptr;
	static MonoDomain* s_new_mono_domain = nullptr;
	static std::string s_core_assembly_path;
	static Ref<Scene> s_scene_context;

	static MonoMethod* GetMethod(MonoImage* image, const std::string& method_description);

	MonoImage* s_app_assembly_image = nullptr;
	MonoImage* s_core_assembly_image = nullptr;

	static MonoMethod* s_exception_method = nullptr;
	static MonoClass* s_entity_class = nullptr;

	static std::unordered_map<std::string, MonoClass*> s_classes;

	struct EntityScriptClass
	{
		std::string Full_name;
		std::string Class_name;
		std::string Namespace_name;

		MonoClass* Class = nullptr;
		MonoMethod* Constructor = nullptr;
		MonoMethod* OnCreateMethod = nullptr;
		MonoMethod* OnDestroyMethod = nullptr;
		MonoMethod* OnUpdateMethod = nullptr;
		
		void InitClassMethods(MonoImage* image)
		{
			Constructor = GetMethod(s_core_assembly_image, "Kablunk.Entity:.ctor(ulong)");
			OnCreateMethod = GetMethod(image, Full_name + ":OnCreate()");
			OnUpdateMethod = GetMethod(image, Full_name + ":OnUpdate(single)");
			OnDestroyMethod = GetMethod(image, Full_name + "OnDestroy()");
		}
	};

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
		if (file_data == NULL)
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

		MonoImageOpenStatus status;
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
		KB_CORE_ASSERT(!s_current_mono_domain, "[C#-ScriptEngine] Mono has already been initialized!");
		mono_set_assemblies_path("mono/lib");
		s_current_mono_domain = mono_jit_init("Kablunk");
	}

	static void ShutdownMono()
	{
		mono_jit_cleanup(s_current_mono_domain);
	}

	static MonoAssembly* LoadAssembly(const std::filesystem::path& path)
	{
		std::string path_as_str = path.string();
		MonoAssembly* assembly = LoadAssemblyFromFile(path_as_str.c_str());
		// can't use loggers here because this will happen before they get initialized
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
			KB_CORE_ERROR("[C#-ScriptEngine] mono_method_desc_new failed ({0})", method_description);

		MonoMethod* method = mono_method_desc_search_in_image(description, image);
		if (!method)
			KB_CORE_WARN("[C#-ScriptEngine] mono_method_desc_search_in_image failed ({0})", method_description);

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
		KB_CORE_WARN("CSharpScriptEngine::OnSceneDestroy() not implemented!");
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
		if (!LoadKablunkRuntimeAssembly(s_core_assembly_path))
			return false;

		if (!LoadAppAssembly(path))
			return false;

		// #TODO loop through entities and reload scripts
	}

	void CSharpScriptEngine::SetSceneContext(const Ref<Scene>& scene)
	{
		s_classes.clear();
		s_scene_context = scene;
	}

	const Kablunk::Ref<Kablunk::Scene>& CSharpScriptEngine::GetCurrentSceneContext()
	{
		return s_scene_context;
	}

	void CSharpScriptEngine::OnCreateEntity(Entity entity)
	{

	}

	void CSharpScriptEngine::OnUpdateEntity(Entity entity, Timestep ts)
	{

	}

	MonoObject* CSharpScriptEngine::Construct(const std::string& full_name, bool call_constructor /*= true*/, void** parameters /*= nullptr*/)
	{

	}

	MonoClass* CSharpScriptEngine::GetCoreClass(const std::string& full_name)
	{

	}

	bool CSharpScriptEngine::ModuleExists(const std::string& module_name)
	{

	}

	std::string CSharpScriptEngine::StripNamespace(const std::string& name_space, const std::string& module_name)
	{

	}

	void CSharpScriptEngine::InitScriptEntity(Entity entity)
	{

	}

	void CSharpScriptEngine::ShutdownScriptEntity(Entity entity, const std::string& module_name)
	{

	}

	void CSharpScriptEngine::InstantiateEntityClass(Entity entity)
	{

	}
}
