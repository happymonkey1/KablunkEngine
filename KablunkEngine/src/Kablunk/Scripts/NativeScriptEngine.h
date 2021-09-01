#ifndef KABLUNK_MODULES_MODULE_H
#define KABLUNK_MODULES_MODULE_H

#include "Kablunk/Core/Core.h"
#include <Kablunk/Scripts/NativeScript.h>

#include <string>
#include <unordered_map>

namespace Kablunk
{

	/*	Native scripts store a create method function pointer inside a map, 
		which can be accessed to instantiate a Scoped script during runtime */
	class NativeScriptEngine
	{
	public:
		using CreateMethodFunc = NativeScript * (*)();

		static bool RegisterScript(const std::string& script_name, CreateMethodFunc create_script);
		static Scope<NativeScript> GetScript(const std::string& script_name);

	private:
		using NativeScriptContainer = std::unordered_map<std::string, CreateMethodFunc>;

		static NativeScriptContainer& GetScriptContainer()
		{
			static NativeScriptContainer m_native_scripts;
			return m_native_scripts;
		}
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
		Kablunk::NativeScriptEngine::RegisterScript(#T, T::Create);
#else
#	define IMPLEMENT_NATIVE_SCRIPT
#	define REGISTER_NATIVE_SCRIPT
#endif

#endif
