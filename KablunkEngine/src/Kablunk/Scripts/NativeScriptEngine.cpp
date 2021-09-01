#include "kablunkpch.h"
#include "Kablunk/Scripts/NativeScriptEngine.h"

#include "Kablunk/Utilities/Parser.h"

namespace Kablunk
{

	bool NativeScriptEngine::RegisterScript(const std::string& script_name, CreateMethodFunc create_script)
	{
		auto& native_scripts = GetScriptContainer();
		auto it = native_scripts.find(script_name);
		if (it == native_scripts.end())
		{
			native_scripts.emplace( script_name, create_script );
			return true;
		}
		else
		{
			KB_CORE_ASSERT(false, "Script already registered!")
			return false;
		}
	}

	// #TODO currently searches for files during runtime, probably better to do at statically with reflection
	Scope<NativeScript> NativeScriptEngine::GetScript(const std::string& script_name)
	{
		auto& native_scripts = GetScriptContainer();
		auto it = native_scripts.find(script_name);
		return it != native_scripts.end() ? Scope<NativeScript>(it->second()) : nullptr;
	}
}
