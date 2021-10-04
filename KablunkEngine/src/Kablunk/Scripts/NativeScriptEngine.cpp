#include "kablunkpch.h"
#include "Kablunk/Scripts/NativeScriptEngine.h"

#include "Kablunk/Utilities/Parser.h"
#include "Kablunk/Utilities/Utilities.h"

#include "Kablunk/Scripts/NativeScriptModule.h"

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

	bool NativeScriptEngine::LoadDLLRuntime(const std::string& dll_name, const std::string& dll_dir)
	{
#if 0
		if (!m_dll_directory_set && false)
		{
			// #TODO move elsewhere
			//std::string dll_directory = "assets/native_scripts";
			//std::wstring dll_directory_wstr = std::wstring{ dll_directory.begin(), dll_directory.end() };

			std::string dll_directory = "C:\\Users\\Gaming Account\\Source\\Repos\\KablunkEngine2020\\KablunkEditor\\assets\\native_scripts\\";
			SetDllDirectoryA(dll_directory.c_str());

			constexpr size_t BUFFER_LEN = 256;
			char dll_dir_buffer[BUFFER_LEN];
			uint32_t get_dll_dir_success = GetDllDirectoryA(BUFFER_LEN, dll_dir_buffer);
			KB_CORE_ASSERT(get_dll_dir_success, "Failed to find dll directory");
			KB_CORE_TRACE("dll directory '{0}'", dll_dir_buffer);

			m_dll_directory_set = true;
		}
#endif

		NativeScriptModule nsc_module = NativeScriptModule{ dll_name, dll_dir };

		if (nsc_module)
		{
			//auto dll_func = nsc_module["DebugPrint"];

			/*if (dll_func != NULL)
			{
				successful_link = true;
				dll_func();
			}*/
		}
		else
		{
			KB_CORE_ERROR("Failed to load dll '{0}'", dll_name);
			return false;
		}

		return true;
	}

}
