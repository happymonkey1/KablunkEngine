#include "kablunkpch.h"

#include "Kablunk/Scripts/NativeScriptModule.h"

namespace Kablunk
{

	NativeScriptModule::NativeScriptModule(const std::string& dll_name, const std::string& dll_path)
		: m_dll_handle{ NULL }
	{
		std::string full_dll_path = dll_path + '/' + dll_name;
		std::replace(full_dll_path.begin(), full_dll_path.end(), '/', '\\'); // only for windows
		m_dll_handle = LoadLibraryA(full_dll_path.c_str());
	}

	NativeScriptModule::~NativeScriptModule()
	{
		if (m_dll_handle != NULL)
			FreeLibrary(m_dll_handle);
	}

}
