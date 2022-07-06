#include "kablunkpch.h"

#include "Kablunk/Plugin/PluginManager.h"

namespace Kablunk
{
	// =======================
	//   Singleton Interface
	// =======================

	void PluginManager::Init() noexcept
	{
		KB_CORE_ASSERT(!s_instance, "Plugin manager already initialized!");

		s_instance = new PluginManager();
	}

	// ===========================
	//   PluginManager Interface
	// ===========================

	PluginManager::PluginManager() noexcept
	{
		KB_CORE_INFO("PluginManager initialized!");
	}

	bool PluginManager::load_plugin(const std::string& plugin_name, bool force_reload /*= false*/) noexcept
	{
		if (!is_plugin_loaded(plugin_name))
		{
			if (!force_reload)
			{
				KB_CORE_WARN("Trying to load plugin that is already loaded!");
				KB_CORE_INFO("  Use force_reload = true to reload plugins in memory.");
				return false;
			}
			else
			{
				// #TODO Plugin reloading
				KB_CORE_ASSERT(false, "Plugin reloading not implemented!");
				
				return false;
			}
		}

		auto pair = m_plugins.emplace(plugin_name, CreateScope<Plugin>(plugin_name));

		// #TODO load function pointers

		return true;
	}

	void PluginManager::unload_plugin(const std::string& plugin_name) noexcept
	{
		if (!is_plugin_loaded(plugin_name))
		{
			KB_CORE_WARN("Trying to unload plugin that is not loaded!");
			return;
		}

		m_plugins.erase(plugin_name);
	}

	WeakRef<Plugin> PluginManager::get_plugin(const std::string& plugin_name) const noexcept
	{
		KB_CORE_ASSERT(is_plugin_loaded(plugin_name), "Plugin is not loaded!");
		return WeakRef<Plugin>(m_plugins.at(plugin_name).get());
	}

	void PluginManager::update() noexcept
	{
		KB_CORE_ASSERT(false, "not implemented!");
	}

}
