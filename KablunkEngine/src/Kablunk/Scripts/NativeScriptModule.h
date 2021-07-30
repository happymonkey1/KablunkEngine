#ifndef KABLUNK_MODULES_MODULE_H
#define KABLUNK_MODULES_MODULE_H

#include "Kablunk/Core/Core.h"
#include <Kablunk/Scripts/NativeScript.h>

#include <string>
#include <unordered_map>

// #TODO generalize module system
namespace Kablunk::Modules
{
	using CreateMethod = NativeScript* (*)();
	

	class NativeScriptModule
	{
	public:
		static bool RegisterScript(const std::string& script_name, CreateMethod create_script);
		static Scope<NativeScript> GetScript(const std::string& script_name);

	private:
		static std::unordered_map<std::string, CreateMethod> m_native_scripts;
	};

}

#define IMPLEMENT_NATIVE_SCRIPT(T) \
	static bool s_registered; \
	static NativeScript* Create() \
	{ \
		return new T(); \
	}

#define REGISTER_NATIVE_SCRIPT(T) bool T::s_registered = \
	Kablunk::Modules::NativeScriptModule::RegisterScript(#T, T::Create);

#endif
