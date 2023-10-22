#include "kablunkpch.h"

#if KB_NATIVE_SCRIPTING

#include "Kablunk/Scripts/NativeScriptModule.h"
#include <iostream>

namespace kb
{

	NativeScriptModule::NativeScriptModule(const std::string& dll_name, const std::string& dll_path)
		: m_dll_name{ dll_name }, m_dll_handle { NULL }
	{
		std::string full_dll_path = dll_path + '/' + dll_name;
		std::replace(full_dll_path.begin(), full_dll_path.end(), '/', '\\'); // only for windows
		
		m_dll_handle = LoadLibraryA(full_dll_path.c_str());
		if (m_dll_handle == NULL)
			KB_CORE_ERROR("Failed to load dll '{0}'", dll_name);
		
	}

	NativeScriptModule::~NativeScriptModule()
	{
		if (m_dll_handle != NULL)
		{
			bool freed = FreeLibrary(m_dll_handle);
			if (!freed)
				KB_CORE_ERROR("DLL '{0}' failed while freeing", m_dll_name);
			else
				KB_CORE_TRACE("DLL '{0}' was freed!", m_dll_name);
		}
	}
}

#endif
