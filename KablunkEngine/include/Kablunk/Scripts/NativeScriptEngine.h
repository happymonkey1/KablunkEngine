#ifndef KABLUNK_MODULES_MODULE_H
#define KABLUNK_MODULES_MODULE_H

#include "Kablunk/Core/Singleton.h"
#include "Kablunk/Scripts/NativeScript.h"

#include <iostream>
#include <string>

/* WARNING, currently need to manually add '__declspec(dllexport)' if project being built is a dll 
 * A function not defined compilation message means the BEGIN_REGISTER_NATIVE_SCRIPTS and END_REGISTER_NATIVE_SCRIPTS
 * are not used/included. 
*/

namespace Kablunk
{
	class Scene;

	class NativeScriptEngine : ISingleton
	{
	public:
		using GetScriptFromRegistryFuncT = INativeScript* (*)(const std::string&);
	public:
		Scope<INativeScript> get_script(const std::string& name);

		WeakRef<Scene> get_scene();
		void set_scene(WeakRef<Scene> scene);

		void set_get_script_from_registry_func(GetScriptFromRegistryFuncT func_ptr) { m_get_script_from_registry = func_ptr; }
	private:
		SINGLETON_CONSTRUCTOR(NativeScriptEngine)
		virtual void init() override;
		virtual void shutdown() override;
	private:
		Scene* m_current_scene = nullptr;
		GetScriptFromRegistryFuncT m_get_script_from_registry = nullptr;

		friend class Singleton<NativeScriptEngine>;
	};
}

// #TODO If these macros are not used, compilation will result in a function definition not found.
//		 Potentially replace with a static map Map<string, NativeScriptInterface*>
//       with a function macro that can insert { string script_names : NativeScriptInterface* script }.
#ifndef KB_DISTRIBUTION
/* Macro to declare a script as a native script, must be used in conjunction with REGISTER_NATIVE_SCRIPT macro. */
// #TODO For some reason visual studio thinks this macro is undefined when using in other projects, look into potential bug.
#	define IMPLEMENT_NATIVE_SCRIPT(T) \
		static Kablunk::INativeScript* Create() \
		{ \
			return new T(); \
		}

/* Register a macro with NativeScriptModule to allow for script loading and use during editor runtime. */
/* WARNING, currently need to manually add '__declspec(dllexport)' if project being built is a dll */
#	define BEGIN_REGISTER_NATIVE_SCRIPTS() \
	extern "C" __declspec(dllexport) Kablunk::INativeScript* get_script_from_registry(const std::string& type_str) \
	{ 
#	define REGISTER_NATIVE_SCRIPT(T) \
		if (Kablunk::Parser::CPP::strip_namespace(std::string{ #T }) == type_str) \
			return T::Create();
#	define END_REGISTER_NATIVE_SCRIPTS() \
		return nullptr; \
	};
#else
#	define IMPLEMENT_NATIVE_SCRIPT
#	define REGISTER_NATIVE_SCRIPT
#endif

#if KB_NATIVE_SCRIPTING

#include "Kablunk/Core/Core.h"
#include <Kablunk/Scripts/NativeScript.h>
#include "Kablunk/Utilities/PlatformUtils.h"
#include "Kablunk/Scripts/NativeScriptModule.h"
#include "Kablunk/Core/SharedMemoryBuffer.h"

#include <string>
#include <unordered_map>
#include <filesystem>

struct IRuntimeObjectSystem;

namespace Kablunk
{
	namespace Internal
	{
		struct NativeScriptRuntimeObject
		{
			NativeScript* ptr;
			ObjectId id;
		};
	}
	

	/*	Native scripts store a create method function pointer inside a map, 
		which can be accessed to instantiate a Scoped script during runtime */
	class NativeScriptEngine : public IObjectFactoryListener
	{
	public:
		using CreateMethodFunc = NativeScript * (*)();

		static NativeScriptEngine* Get()
		{
			if (!s_native_script_engine)
				Init();
			return s_native_script_engine;
		}

		static void Init();
		static void Shutdown();
		static bool AreScriptsCompiling();
		
	public:
		NativeScriptEngine();
		~NativeScriptEngine();
		
		[[deprecated("Replaced by rccpp")]]
		bool RegisterScript(const std::string& script_name, CreateMethodFunc create_script);
		[[deprecated("Replaced by rccpp")]]
		Scope<NativeScript> get_script(const std::string& script_name);

		[[deprecated("Replaced by rccpp")]]
		bool LoadDLLRuntime(const std::string& dll_name, const std::string& dll_dir);

		NativeScript** AddScript(const std::string& name, const std::filesystem::path& filepath);
		virtual void OnConstructorsAdded() override;
		void OnUpdate(Timestep ts);

	private:
		inline static NativeScriptEngine* s_native_script_engine;
	private:
		using NativeScriptContainer = std::unordered_map<std::string, Internal::NativeScriptRuntimeObject>;
		
		bool m_dll_directory_set = false;

		// RCCPP
		ICompilerLogger* m_compiler_logger;
		IRuntimeObjectSystem* m_runtime_object_system;
		NativeScriptContainer m_native_scripts;
	};

	
}




#endif

#endif
