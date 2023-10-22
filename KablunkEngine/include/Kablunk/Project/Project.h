#ifndef KABLUNK_PROJECT_PROJECT_H
#define KABLUNK_PROJECT_PROJECT_H

#include "Kablunk/Core/Core.h"

#include <string>
#include <filesystem>

namespace kb
{

	struct ProjectConfig
	{
		std::string Name;

		std::string Asset_directory;
		std::string Native_script_modules_path;
		std::string Native_script_default_namespace;
		std::string CSharp_script_modules_path;
		std::string CSharp_script_default_namespace;

		// #TODO actually implement this
		bool Reload_native_script_assemblies_on_play;
		bool Reload_csharp_script_assemblies_on_play;

		std::string Start_scene;

		std::string Project_filename;
		std::string Project_directory;
	};

	class Project : public RefCounted
	{
	public:
		Project();
		~Project();

		const ProjectConfig& GetConfig() const { return m_config; }

		const void set_project_name(const std::string& new_name) { m_config.Name = new_name; }
		const std::string& get_project_name() const { return m_config.Name; }
		std::filesystem::path get_project_directory() const { return m_config.Project_directory; }
		std::filesystem::path get_asset_directory() const { return m_config.Asset_directory; }
		std::filesystem::path get_asset_directory_path() const { return get_project_directory() / get_asset_directory(); }

		const std::string& get_start_scene_name() const { return m_config.Start_scene; }
		const void set_start_scene_name(const std::string& new_name) { m_config.Start_scene = new_name; }

		std::filesystem::path get_native_script_module_path()
		{
			return std::filesystem::path{ m_config.Project_directory }
			/ std::filesystem::path{ m_config.Native_script_modules_path };
		}

		std::string get_native_script_module_file_name() { return get_project_name() + ".dll"; }
		std::filesystem::path get_native_script_module_file_path()
		{
			return get_native_script_module_path() / get_native_script_module_file_name();
		}

		std::filesystem::path get_csharp_script_module_path()
		{
			return std::filesystem::path{ m_config.Project_directory }
			/ std::filesystem::path{ m_config.CSharp_script_modules_path };
		}

		std::string get_csharp_script_module_file_name() { return get_project_name() + ".dll"; }
		std::filesystem::path get_csharp_script_module_file_path()
		{
			return get_csharp_script_module_path() / get_csharp_script_module_file_name();
		}
	private:
		ProjectConfig m_config;

		friend class ProjectSerializer;
		friend class ProjectPropertiesPanel;
		friend class ProjectManager;
	};

}

#endif
