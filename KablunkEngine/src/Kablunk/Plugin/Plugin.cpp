#include "kablunkpch.h"
#include "Kablunk/Plugin/Plugin.h"

namespace Kablunk
{

	Plugin::Plugin(const std::string& dll_name, const std::filesystem::path& dll_path) noexcept
		: m_dll_name{ dll_name }, m_path{ dll_path } 
	{
		std::string dll_path_as_str = dll_path.string();
		// don't load dllmain or other references so we aren't worried about DLL hijacks.
		//m_handle = LoadLibraryExA(dll_path_as_str.c_str(), nullptr, DONT_RESOLVE_DLL_REFERENCES);
		m_handle = LoadLibraryA(dll_path_as_str.c_str());

		if (!is_loaded())
			KB_CORE_ASSERT("failed to load dll '{}'", dll_name);

		KB_CORE_INFO("Loaded dll '{}'", dll_name);
	}

	Plugin::~Plugin() noexcept
	{
		if (!is_loaded())
			return;
		
		if (FreeLibrary(m_handle))
			KB_CORE_INFO("Released dll '{}'", m_dll_name);
		else
			KB_CORE_ERROR("Failed to release dll '{}'", m_dll_name);
	}
	

	void Plugin::load_function_from_plugin(const std::string& name) noexcept
	{
		KB_CORE_ASSERT(is_loaded(), "dll was not loaded into memory!");

		if (m_void_funcs_cache.find(name) != m_void_funcs_cache.end())
		{
			KB_CORE_WARN("Function '{}' from dll '{}' is already loaded! Exiting early.", name, m_dll_name);
			return;
		}

		// #TODO c++ style casting
		void_func_t dll_func = (void_func_t)(GetProcAddress(m_handle, name.c_str()));

		if (!dll_func)
		{
			KB_CORE_ASSERT("Failed to load function '{}' from dll '{}'", name, m_dll_name);
			return;
		}

		KB_CORE_INFO("Successfully loaded function '{}' from dll '{}'", name, m_dll_name);

		m_void_funcs_cache.insert({ name, dll_func });
	}

}
