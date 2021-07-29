#include "kablunkpch.h"
#include "Kablunk/Scripts/NativeScriptModule.h"

#include "Kablunk/Utilities/Parser.h"

namespace Kablunk::Modules
{
	void NativeScriptModule::RegisterScript(const std::string& script_name, NativeScript script)
	{
		m_native_scripts.insert({ script_name, CreateRef<NativeScript>(script) });
	}

	// #TODO currently searches for files during runtime, probably better to do at statically with reflection
	Ref<NativeScript> NativeScriptModule::GetScript(const std::string& script_name)
	{
		auto it = m_native_scripts.find(script_name);
		return it != m_native_scripts.end() ? it->second : Ref<NativeScript>{};
	}
}
