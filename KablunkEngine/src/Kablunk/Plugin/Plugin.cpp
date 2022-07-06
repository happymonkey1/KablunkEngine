#include "kablunkpch.h"
#include "Kablunk/Plugin/Plugin.h"

namespace Kablunk
{

	Plugin::Plugin(const std::string& dll_name) noexcept
		: m_dll_name{ dll_name }
	{
		// cast string to wstring for use with LoadLibrary.
		std::wstring str_casted = std::wstring{ dll_name.begin(), dll_name.end() };
		m_handle = LoadLibrary(str_casted.c_str());

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
		VoidFunc dll_func = (VoidFunc)(GetProcAddress(m_handle, name.c_str()));

		if (!dll_func)
		{
			KB_CORE_ASSERT("Failed to load function '{}' from dll '{}'", name, m_dll_name);
			return;
		}

		KB_CORE_INFO("Successfully loaded function '{}' from dll '{}'", name, m_dll_name);

		m_void_funcs_cache.insert({ name, dll_func });
	}

}
