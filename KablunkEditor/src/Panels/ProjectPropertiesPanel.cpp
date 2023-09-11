#include "Panels/ProjectPropertiesPanel.h"

#include "Kablunk/Project/ProjectSerializer.h"
#include "Kablunk/Imgui/ImGuiWrappers.h"

#include <imgui/imgui.h>
#include <filesystem>

namespace kb
{
	constexpr size_t MAX_PATH_BUFFER_SIZE = 256;
	static char s_scene_path_buffer[MAX_PATH_BUFFER_SIZE];
	static bool s_serialize_project = false;

	ProjectPropertiesPanel::ProjectPropertiesPanel(const ref<Project>& project)
		: m_project{ project }, m_default_scene{ project != nullptr ? project->get_start_scene_name() : "Untitled Scene"}
	{
		memset(s_scene_path_buffer, 0, MAX_PATH_BUFFER_SIZE);
	}

	void ProjectPropertiesPanel::OnImGuiRender(bool& show)
	{
		if (!show)
			return;

		ImGui::Begin("Project Settings", &show);

		UI_GeneralSettings();

		ImGui::End();

		if (s_serialize_project)
		{
			ProjectSerializer serializer{ m_project };
			serializer.Serialize(std::filesystem::path{ m_project->get_project_directory() / m_project->get_project_name() }.string());
			s_serialize_project = false;
		}
	}

	void ProjectPropertiesPanel::UI_GeneralSettings()
	{
		ImGui::PushID("GeneralSettings");

		UI::BeginProperties();

		UI::PushItemDisabled();
		
		UI::Property("Name", m_project->m_config.Name);
		UI::Property("Asset Directory", m_project->get_asset_directory().string());
		UI::Property("Native Script Modules Path", m_project->get_native_script_module_file_path().string());
		UI::Property("Project Directory", m_project->get_project_directory().string());

		UI::PopItemDisabled();

		// #TODO startup scene selection;
		//UI::Property("Startup Scene", m_project->GetStartSceneName());
		std::string default_scene_path;
		if (m_project->get_start_scene_name() != "$SCENE_NAME$")
			default_scene_path = m_project->get_asset_directory().string() + "/scenes/" + m_project->get_start_scene_name();
		else
			default_scene_path = "no default scene set!";

		if (UI::PropertyFolderPathWithButton("Startup Scene", default_scene_path.c_str(), MAX_PATH_BUFFER_SIZE))
		{
			std::filesystem::path new_start_scene = FileDialog::OpenFile("Kablunk Scene (*.kablunkscene)\0*.kablunkscene\0");
			m_project->set_start_scene_name(new_start_scene.filename().string());
		}

		UI::EndProperties();

		ImGui::PopID();
	}

}
