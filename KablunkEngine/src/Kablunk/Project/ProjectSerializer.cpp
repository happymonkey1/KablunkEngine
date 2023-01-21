#include "kablunkpch.h"

#include "Kablunk/Project/ProjectSerializer.h"
#include "Kablunk/Scene/YamlSpecializedSerialization.h"

#include <yaml-cpp/yaml.h>

#include <filesystem>


namespace Kablunk
{
	
	ProjectSerializer::ProjectSerializer(const IntrusiveRef<Project>& project)
		: m_project{ project }
	{

	}

	void ProjectSerializer::Serialize(const std::string& filepath)
	{
		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "Project" << YAML::Value;

		out << YAML::BeginMap;

		const ProjectConfig& config = m_project->GetConfig();
		out << YAML::Key << "Name" << YAML::Value << config.Name;
		out << YAML::Key << "Asset_directory" << YAML::Value << config.Asset_directory;
		out << YAML::Key << "Native_script_modules_path" << YAML::Value << config.Native_script_modules_path;
		out << YAML::Key << "Native_script_default_namespace" << YAML::Value << config.Native_script_default_namespace;
		out << YAML::Key << "Reload_native_script_assemblies_on_play" << YAML::Value << config.Reload_native_script_assemblies_on_play;
		out << YAML::Key << "CSharp_script_modules_path" << YAML::Value << config.CSharp_script_modules_path;
		out << YAML::Key << "CSharp_script_default_namespace" << YAML::Value << config.CSharp_script_default_namespace;
		out << YAML::Key << "Reload_csharp_script_assemblies_on_play" << YAML::Value << config.Reload_csharp_script_assemblies_on_play;
		out << YAML::Key << "Start_scene" << YAML::Value << config.Start_scene;
		out << YAML::EndMap;

		out << YAML::EndMap;

		std::ofstream fout(filepath);
		fout << out.c_str();
	}

	bool ProjectSerializer::Deserialize(const std::string& filepath)
	{
		std::ifstream stream{ filepath };
		KB_CORE_ASSERT(!stream.bad(), "Could not deserialize project!");
		std::stringstream ss;
		ss << stream.rdbuf();

		YAML::Node data;
		try
		{
			data = YAML::Load(ss.str());
			if (!data["Project"])
				return false;
		}
		catch (YAML::ParserException& e)
		{
			KB_CORE_ERROR("Failed to deserialize project file '{0}'", filepath);
			KB_CORE_ERROR("    {0}", e.msg);
			return false;
		}

		YAML::Node root = data["Project"];
		if (!root["Name"])
			return false;

		auto& config = m_project->m_config;
		config.Name = root["Name"].as<std::string>();

		config.Asset_directory = root["Asset_directory"].as<std::string>();
		config.Native_script_modules_path = root["Native_script_modules_path"].as <std::string>();
		config.Native_script_default_namespace = root["Native_script_default_namespace"].as<std::string>();
		config.Reload_native_script_assemblies_on_play = root["Reload_native_script_assemblies_on_play"].as<bool>();
		config.CSharp_script_modules_path = root["CSharp_script_modules_path"].as<std::string>();
		config.CSharp_script_default_namespace = root["CSharp_script_default_namespace"].as<std::string>();
		config.Reload_csharp_script_assemblies_on_play = root["Reload_csharp_script_assemblies_on_play"].as<bool>();
		config.Start_scene = root["Start_scene"].as<std::string>();

		std::filesystem::path project_path = filepath;
		config.Project_filename = project_path.filename().string();
		config.Project_directory = project_path.parent_path().string();

		return true;
	}

}
