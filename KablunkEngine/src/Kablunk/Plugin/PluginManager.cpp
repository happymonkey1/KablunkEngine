#include "kablunkpch.h"

#include "Kablunk/Plugin/PluginManager.h"
#include "Kablunk/Scripts/NativeScript.h"
#include "Kablunk/Scripts/NativeScriptEngine.h"

namespace Kablunk
{

	// ===========================
	//   PluginManager Interface
	// ===========================

	void PluginManager::init()
	{

	}

	void PluginManager::shutdown()
	{
		for (auto& [name, plugin] : m_plugins)
			plugin->unload();

		
	}

	WeakRef<Plugin> PluginManager::load_plugin(const std::string& plugin_name, const std::filesystem::path& plugin_path, PluginType plugin_type, bool force_reload /*= false*/) noexcept
	{
		if (is_plugin_loaded(plugin_name))
		{
			if (!force_reload)
			{
				KB_CORE_WARN("Trying to load plugin that is already loaded!");
				KB_CORE_INFO("  Use force_reload = true to reload plugins in memory.");
				return nullptr;
			}
			else
				return reload_plugin(plugin_name, plugin_path, plugin_type);
		}

		auto pair = m_plugins.emplace(plugin_name, IntrusiveRef<Plugin>::Create(plugin_name, plugin_path, plugin_type));
		auto plugin = WeakRef<Plugin>(m_plugins[plugin_name].get());

		if (!plugin || !plugin->is_loaded())
		{
			KB_CORE_ASSERT(false, "Plugin {} failed to load", plugin_name);
			return nullptr;
		}

		plugin->init();

		return plugin;
	}

	WeakRef<Plugin> PluginManager::reload_plugin(const std::string& plugin_name, const std::filesystem::path& plugin_path, PluginType plugin_type) noexcept
	{
		KB_CORE_ASSERT(is_plugin_loaded(plugin_name), "Trying to reload {}, which is not loaded!", plugin_name);

		get_plugin(plugin_name)->unload();

		m_plugins[plugin_name] = IntrusiveRef<Plugin>::Create(plugin_name, plugin_path, plugin_type);

		WeakRef<Plugin> plugin = get_plugin(plugin_name);
		plugin->init();

		return plugin;
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
		WeakRef<Plugin> plugin = m_plugins.at(plugin_name);

		KB_CORE_ASSERT(plugin, "Plugin is not valid!");
		return plugin;
	}

	void PluginManager::update() noexcept
	{
		KB_CORE_ASSERT(false, "not implemented!");
	}

}
