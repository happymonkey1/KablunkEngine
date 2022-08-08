#pragma once
#ifndef KABLUNK_PLUGIN_PLUGIN_MANAGER_H
#define KABLUNK_PLUGIN_PLUGIN_MANAGER_H

#include "Kablunk/Core/Core.h"

#include "Kablunk/Plugin/Plugin.h"

#include <string>
#include <unordered_map>

namespace Kablunk
{

	class PluginManager : public ISingleton
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
	private:
		SINGLETON_CONSTRUCTOR(PluginManager)
		PluginManager(const PluginManager&) = delete;
		PluginManager(PluginManager&&) = delete;

		virtual void init();
		virtual void shutdown();

		WeakRef<Plugin> reload_plugin(const std::string& plugin_name, const std::filesystem::path& plugin_path, PluginType plugin_type) noexcept;
	private:
		std::unordered_map<std::string, IntrusiveRef<Plugin>> m_plugins;

		SINGLETON_FRIEND(PluginManager)
	};

}

#endif
