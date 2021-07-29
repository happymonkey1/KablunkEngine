#ifndef KABLUNK_MODULES_MODULE_H
#define KABLUNK_MODULES_MODULE_H

#include "Kablunk/Core/Core.h"
#include <Kablunk/Scripts/NativeScript.h>
#include <string>
#include <unordered_map>

// #TODO generalize module system
namespace Kablunk::Modules
{
	class NativeScriptModule
	{
	public:
		NativeScriptModule(const std::string& module_name)
			: m_name{ module_name }
		{

		};
		virtual ~NativeScriptModule() = default;

		void RegisterScript(const std::string& script_name, NativeScript script);
		Ref<NativeScript> GetScript(const std::string& script_name);

	private:
		std::string m_name;
		std::unordered_map<std::string, Ref<NativeScript>> m_native_scripts;
	};

	static NativeScriptModule s_native_script_module{ "native scripts" };
}

#define REGISTER_NATIVE_SCRIPT(name) [this]() { Kablunk::Modules::s_native_script_module.RegisterScript(#name, *this); }

#endif
