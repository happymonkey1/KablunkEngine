#ifndef KABLUNK_MODULES_MODULE_H
#define KABLUNK_MODULES_MODULE_H

#include "Kablunk/Core/Core.h"
#include <Kablunk/Scripts/NativeScript.h>
#include "Kablunk/Utilities/PlatformUtils.h"
#include "Kablunk/Scripts/NativeScriptModule.h"
#include "Kablunk/Core/SharedMemoryBuffer.h"

#include "RCCPP/RuntimeObjectSystem/IObjectFactorySystem.h"
#include "RCCPP/RuntimeObjectSystem/ObjectInterface.h"
#include "RCCPP/RuntimeCompiler/AUArray.h"


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
		Scope<NativeScript> GetScript(const std::string& script_name);

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


#ifndef KB_DISTRIBUTION
/* Macro to declare a script as a native script, must be used in conjunction with REGISTER_NATIVE_SCRIPT macro. */
// #TODO For some reason visual studio thinks this macro is undefined when using in other projects, look into potential bug.
#	define IMPLEMENT_NATIVE_SCRIPT(T) \
		static bool s_registered; \
		static NativeScript* Create() \
		{ \
			return new T(); \
		}

/* Register a macro with NativeScriptModule to allow for script loading and use during editor runtime. */
#	define REGISTER_NATIVE_SCRIPT(T) bool T::s_registered = \
		Kablunk::NativeScriptEngine::Get()->RegisterScript(#T, T::Create);
#else
#	define IMPLEMENT_NATIVE_SCRIPT
#	define REGISTER_NATIVE_SCRIPT
#endif

#endif
