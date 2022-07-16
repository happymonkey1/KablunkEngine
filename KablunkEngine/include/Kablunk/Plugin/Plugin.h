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
	enum class PluginType : u8
	{
		NativeScript = 0,
		Other 
	};

	class Plugin : public RefCounted
	{
	public:
		using void_func_t = void(*)();
	public:
		explicit Plugin(const std::string& dll_name, const std::filesystem::path& dll_path, PluginType plugin_type) noexcept;
		~Plugin() noexcept;
		
		void init() noexcept;
		bool is_loaded() const noexcept { return m_handle && m_handle != INVALID_HANDLE_VALUE; }
		void unload() noexcept;

		PluginType get_plugin_type() const { return m_plugin_type; }

		// #TODO figure out how to cache templated function pointers
		template <typename FuncT>
		FuncT get_function(const std::string& func_name) noexcept;
	private:
		void load_native_script_entry_point();

	private:
		HINSTANCE m_handle = nullptr;
		std::string m_dll_name = "INV_DLL_NAME";
		std::filesystem::path m_path;
		PluginType m_plugin_type;
	};


	// Allow client code to get an arbitrary function pointer from the dll. Does not cache loaded functions, re-loading the function pointer on subsequent calls.
	template <typename FuncT>
	FuncT Plugin::get_function(const std::string& func_name) noexcept
	{
		KB_CORE_ASSERT(is_loaded(), "dll was not loaded into memory!");

		// #TODO c++ style casting
		FuncT dll_func = (FuncT)(GetProcAddress(m_handle, func_name.c_str()));

		if (!dll_func)
		{
			KB_CORE_ASSERT(false, "Failed to load function '{}' from dll '{}'", func_name, m_dll_name);
			return nullptr;
		}

		KB_CORE_INFO("Successfully loaded function '{}' from dll '{}'", func_name, m_dll_name);

		return dll_func;
	}

}

#endif
