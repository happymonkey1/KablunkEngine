#include "kablunkpch.h"
#include "Kablunk/Scripts/NativeScriptModule.h"

#include "Kablunk/Utilities/Parser.h"

namespace Kablunk::Modules
{
	std::unordered_map<std::string, CreateMethod> NativeScriptModule::m_native_scripts;

	bool NativeScriptModule::RegisterScript(const std::string& script_name, CreateMethod create_script)
	{
		auto it = m_native_scripts.find(script_name);
		if (it == m_native_scripts.end())
		{
			m_native_scripts.emplace( script_name, create_script );
			return true;
		}
		else
		{
			KB_CORE_ASSERT(false, "Script already registered!")
			return false;
		}
	}

	// #TODO currently searches for files during runtime, probably better to do at statically with reflection
	Scope<NativeScript> NativeScriptModule::GetScript(const std::string& script_name)
	{
		auto it = m_native_scripts.find(script_name);
		return it != m_native_scripts.end() ? Scope<NativeScript>(it->second()) : nullptr;
	}
}
