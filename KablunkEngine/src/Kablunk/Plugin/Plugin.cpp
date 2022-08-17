#include "kablunkpch.h"
#include "Kablunk/Plugin/Plugin.h"

#include "Kablunk/Scripts/NativeScriptEngine.h"

namespace Kablunk
{

	Plugin::Plugin(const std::string& dll_name, const std::filesystem::path& dll_path, PluginType plugin_type) noexcept
		: m_dll_name{ dll_name }, m_path{ dll_path }, m_plugin_type{ plugin_type }
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
		if (is_loaded())
			unload();
	}
	

	void Plugin::init() noexcept
	{
		// load GetScriptFromRegistry function ptr
		switch (m_plugin_type)
		{
			case PluginType::NativeScript:
			{
				load_native_script_entry_point();
				break;
			}
		}
	}

	void Plugin::unload() noexcept
	{
		if (!is_loaded())
			return;

		if (FreeLibrary(m_handle))
			KB_CORE_INFO("Released dll '{}'", m_dll_name);
		else
			KB_CORE_ERROR("Failed to release dll '{}'", m_dll_name);

		m_handle = nullptr;

		switch (m_plugin_type)
		{
			case PluginType::NativeScript:
			{
				NativeScriptEngine::get().set_get_script_from_registry_func(nullptr);
				break;
			}
		}
	}

	void Plugin::load_native_script_entry_point()
	{
		const char* entry_point_func_c_str = "get_script_from_registry";
		KB_CORE_INFO("loading native script module '{}' entry point '{}'", m_dll_name, entry_point_func_c_str);

		NativeScriptEngine::GetScriptFromRegistryFuncT get_script_from_registry_func = get_function<NativeScriptEngine::GetScriptFromRegistryFuncT>(entry_point_func_c_str);

		KB_CORE_ASSERT(get_script_from_registry_func, "{} failed to load get_script_from_registry!", m_dll_name);

		NativeScriptEngine::get().set_get_script_from_registry_func(get_script_from_registry_func);
	}

}
