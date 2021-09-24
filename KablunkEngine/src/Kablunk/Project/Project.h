#ifndef KABLUNK_PROJECT_PROJECT_H
#define KABLUNK_PROJECT_PROJECT_H

#include "Kablunk/Core/Core.h"

#include <string>
#include <filesystem>

namespace Kablunk
{

	struct ProjectConfig
	{
		std::string Name;

		std::string Asset_directory;
		std::string Native_script_modules_path;
		std::string Native_script_default_namespace;

		// #TODO actually implement this
		bool Reload_native_script_assemblies_on_play;

		std::string Start_scene;

		std::string Project_filename;
		std::string Project_directory;
	};
	
	class Project
	{
	public:
		Project();
		~Project();

		const ProjectConfig& GetConfig() const { return m_config; }

		static Ref<Project> GetActive() { return s_active_project; }
		static void SetActive(const Ref<Project>& project);

		static const void SetProjectName(const std::string& new_name) { s_active_project->m_config.Name = new_name; }
		static const std::string& GetProjectName() { return s_active_project->GetConfig().Name; }
		static std::filesystem::path GetProjectDirectory() { return s_active_project->GetConfig().Project_directory; }
		static std::filesystem::path GetAssetDirectory() { return s_active_project->GetConfig().Asset_directory; }

		static std::filesystem::path GetNativeScriptModulePath() 
		{ 
			return std::filesystem::path{ s_active_project->GetConfig().Project_directory } 
			/ std::filesystem::path{ s_active_project->GetConfig().Native_script_modules_path };
		}
		static std::string GetNativeScriptModuleFileName() { return s_active_project->GetProjectName() + ".dll"; }
		static std::filesystem::path GetNativeScriptModuleFilePath()
		{ 
			return GetNativeScriptModulePath() / GetNativeScriptModuleFileName();
		}

		static const std::string& GetStartSceneName() { return s_active_project->GetConfig().Start_scene; }

	private:
		ProjectConfig m_config;

		inline static Ref<Project> s_active_project = nullptr;

		friend class ProjectSerializer;
		friend class ProjectPropertiesPanel;
	};

}

#endif
