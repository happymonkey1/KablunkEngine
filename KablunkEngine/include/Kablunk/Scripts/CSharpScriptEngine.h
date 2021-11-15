#ifndef KABLUNK_SCRIPTS_CSHARP_SCRIPT_ENGINE_H
#define KABLUNK_SCRIPTS_CSHARP_SCRIPT_ENGINE_H

#include "Kablunk/Core/Core.h"
#include "Kablunk/Core/Timestep.h"

#include "Kablunk/Scene/Components.h"
#include "Kablunk/Scene/Entity.h"

#include <filesystem>
#include <string>

extern "C"
{
	typedef struct _MonoObject MonoObject;
	typedef struct _MonoClass MonoClass;
}

namespace Kablunk
{
	class CSharpScriptEngine
	{
	public:
		static void Init(const std::filesystem::path& assembly_path);
		static void Shutdown();

		static void OnSceneDestroy(uuid::uuid64 scene_id);

		static bool LoadKablunkRuntimeAssembly(const std::filesystem::path& path);
		static bool LoadAppAssembly(const std::filesystem::path& path);
		static bool ReloadAssembly(const std::filesystem::path& path);

		static void SetSceneContext(const Ref<Scene>& scene);
		static const Ref<Scene>& GetCurrentSceneContext();

		static void OnCreateEntity(Entity entity);
		static void OnUpdateEntity(Entity entity, Timestep ts);

		static MonoObject* Construct(const std::string& full_name, bool call_constructor = true, void** parameters = nullptr);
		static MonoClass* GetCoreClass(const std::string& full_name);

		static bool ModuleExists(const std::string& module_name);
		static std::string StripNamespace(const std::string& name_space, const std::string& module_name);

		static void InitScriptEntity(Entity entity);
		static void ShutdownScriptEntity(Entity entity, const std::string& module_name);
		static void InstantiateEntityClass(Entity entity);
	};
}

#endif
