#ifndef KABLUNK_PLUGIN_PLUGIN_H
#define KABLUNK_PLUGIN_PLUGIN_H

#include "Kablunk/Core/Core.h"

#include <string>
#include <unordered_map>
#include <filesystem>

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
		using void_func_t = void(*)();
	public:
		explicit Plugin(const std::string& dll_name, const std::filesystem::path& dll_path) noexcept;
		~Plugin() noexcept;
		
		bool is_loaded() const noexcept { return m_handle && m_handle != INVALID_HANDLE_VALUE; }
	// #TODO private:

		void load_function_from_plugin(const std::string& name) noexcept;

		// #TODO figure out how to cache templated function pointers
		template <typename FuncT>
		FuncT get_function(const std::string& func_name) noexcept;
		
	private:
		HINSTANCE m_handle = nullptr;
		std::string m_dll_name = "INV_DLL_NAME";
		std::filesystem::path m_path;

		std::unordered_map<std::string, void_func_t> m_void_funcs_cache;
	};


	// Allow client code to get an arbitrary function pointer from the dll. Does not cache loaded functions, re-loading the function pointer on subsequent calls.
	template <typename FuncT>
	FuncT Plugin::get_function(const std::string& func_name) noexcept
	{
		KB_CORE_ASSERT(m_handle, "dll was not loaded into memory!");

		/* #TODO figure out how to check and store function pointers of all types
		if (m_void_funcs_cache.find(func_name) != m_void_funcs_cache.end())
		{
			KB_CORE_WARN("Function '{}' from dll '{}' is already loaded! Exiting early.", func_name, m_dll_name);
			return m_void_funcs_cache.at(func_name);
		}*/

		// #TODO c++ style casting
		FuncT dll_func = (FuncT)(GetProcAddress(m_handle, func_name.c_str()));

		if (!dll_func)
		{
			KB_CORE_ASSERT(false, "Failed to load function '{}' from dll '{}'", func_name, m_dll_name);
			return nullptr;
		}

		KB_CORE_INFO("Successfully loaded function '{}' from dll '{}'", func_name, m_dll_name);

		//m_void_funcs_cache.insert({ func_name, dll_func });
		return dll_func;
	}

}

#endif
