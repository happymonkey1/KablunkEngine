#pragma once
#ifndef KABLUNK_PLUGIN_PLUGIN_MANAGER_H
#define KABLUNK_PLUGIN_PLUGIN_MANAGER_H

#include "Kablunk/Core/Core.h"

#include "Kablunk/Plugin/Plugin.h"

#include <string>
#include <unordered_map>

namespace kb
{

	class PluginManager
	{
	public:
		~PluginManager() = default;

		// Load plugin into memory, will fail if plugin is already loaded.
		WeakRef<Plugin> load_plugin(const std::string& plugin_name, const std::filesystem::path& plugin_path, PluginType plugin_type, bool force_reload = false) noexcept;
		// Try to load plugin into memory, will return loaded plugin if already loaded.
		WeakRef<Plugin> try_load_plugin(const std::string& plugin_name, const std::filesystem::path& plugin_path, PluginType plugin_type) noexcept;
		bool is_plugin_loaded(const std::string& plugin_name) const noexcept { return m_plugins.find(plugin_name) != m_plugins.end(); };
		void unload_plugin(const std::string& plugin_name) noexcept;

		WeakRef<Plugin> get_plugin(const std::string& plugin_name) const noexcept;

		void update() noexcept;

		virtual void init();
		virtual void shutdown();

		WeakRef<Plugin> reload_plugin(const std::string& plugin_name, const std::filesystem::path& plugin_path, PluginType plugin_type) noexcept;

		SINGLETON_GET_FUNC(PluginManager)
	private:
		kb::unordered_flat_map<std::string, ref<Plugin>> m_plugins;
	};

}

#endif
