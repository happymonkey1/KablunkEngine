#pragma once
#ifndef KABLUNK_PLUGIN_PLUGIN_MANAGER_H
#define KABLUNK_PLUGIN_PLUGIN_MANAGER_H

#include "Kablunk/Core/Core.h"

#include "Kablunk/Plugin/Plugin.h"

#include <string>
#include <unordered_map>

namespace Kablunk
{

	class PluginManager
	{
	public:
		~PluginManager() = default;
		// =======================
		//   Singleton Interface
		// =======================

		static void Init() noexcept;
		static PluginManager* Get() noexcept { KB_CORE_ASSERT(s_instance, "Plugin Manager instance is not initialized!"); return s_instance; }
		static void Shutdown() noexcept { KB_CORE_ASSERT(s_instance, "Plugin Manager instance is not initialized!"); delete s_instance; }

		// ===========================
		//   PluginManager Interface
		// ===========================

		WeakRef<Plugin> load_plugin(const std::string& plugin_name, const std::filesystem::path& plugin_path, bool force_reload = false) noexcept;
		bool is_plugin_loaded(const std::string& plugin_name) const noexcept { return m_plugins.find(plugin_name) != m_plugins.end(); };
		void unload_plugin(const std::string& plugin_name) noexcept;

		WeakRef<Plugin> get_plugin(const std::string& plugin_name) const noexcept;


		void update() noexcept;
	private:
		explicit PluginManager() noexcept;
		PluginManager(const PluginManager&) = delete;
		PluginManager(PluginManager&&) = delete;
	private:
		static PluginManager* s_instance;

		std::unordered_map<std::string, Scope<Plugin>> m_plugins;
	};

}

#endif
