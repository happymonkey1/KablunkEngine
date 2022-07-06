#ifndef KABLUNK_PLUGIN_PLUGIN_H
#define KABLUNK_PLUGIN_PLUGIN_H

#include "Kablunk/Core/Core.h"

#include <string>
#include <unordered_map>

#ifdef KB_PLATFORM_WINDOWS
#	include <windows.h>
#else
#	error "Plugins are not supported on platforms other than windows!"
#endif

namespace Kablunk
{

	class Plugin
	{
	public:
		using VoidFunc = void(*)();
	public:
		explicit Plugin(const std::string& dll_name) noexcept;
		~Plugin() noexcept;
		
		bool is_loaded() const noexcept { return m_handle && m_handle != INVALID_HANDLE_VALUE; }
	private:

		void load_function_from_plugin(const std::string& name) noexcept;

		// #TODO figure out how to cache templated function pointers
		template <typename FuncT>
		FuncT get_function(const std::string& func_name) noexcept;
		
	private:
		HINSTANCE m_handle = nullptr;
		std::string m_dll_name = "INV_DLL_NAME";

		std::unordered_map<std::string, VoidFunc> m_void_funcs_cache;
	};


	// Allow client code to get an arbitrary function pointer from the dll. Does not cache loaded functions, re-loading the function pointer on subsequent calls.
	template <typename FuncT>
	FuncT Plugin::get_function(const std::string& func_name) noexcept
	{
		KB_CORE_ASSERT(m_handle, "dll was not loaded into memory!");

		if (m_void_funcs_cache.find(name) != m_void_funcs_cache.end())
		{
			KB_CORE_WARN("Function '{}' from dll '{}' is already loaded! Exiting early.", name, m_dll_name);
			return;
		}

		// #TODO c++ style casting
		FuncT dll_func = (FuncT)(GetProcAddress(m_handle, name.c_str()));

		if (!dll_func)
		{
			KB_CORE_ASSERT("Failed to load function '{}' from dll '{}'", name, m_dll_name);
			return;
		}

		KB_CORE_INFO("Successfully loaded function '{}' from dll '{}'", name, m_dll_name);

		m_void_funcs_cache.insert({ name, dll_func });
		return dll_func;
	}

}

#endif
