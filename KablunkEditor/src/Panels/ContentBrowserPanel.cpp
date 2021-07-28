#include "Panels/ContentBrowserPanel.h"

#include <imgui/imgui.h>
#include <glm/gtc/type_ptr.hpp>

#include "imgui/imgui_internal.h"

#include <filesystem>

namespace Kablunk
{
	static const std::filesystem::path s_asset_path = "assets";

	ContentBrowserPanel::ContentBrowserPanel()
		: m_current_directory{ s_asset_path }
	{

	}

	void ContentBrowserPanel::OnImGuiRender()
	{
		ImGui::Begin("Content Browser");

		if (m_current_directory != s_asset_path)
		{
			if (ImGui::Button("<<<"))
				m_current_directory = m_current_directory.parent_path();
			
		}

		for (auto& directory_entry : std::filesystem::directory_iterator{ m_current_directory })
		{
			const auto& path = directory_entry.path();
			auto relative_path = std::filesystem::relative(path, s_asset_path);
			auto relative_path_string = relative_path.filename().string();

			if (directory_entry.is_directory())
			{
				if (ImGui::Button(relative_path_string.c_str()))
				{
					m_current_directory /= path.filename();
				}
			}
			else
			{
				if (ImGui::Button(relative_path_string.c_str()))
				{

				}
			}
		}

		ImGui::End();
	}
}
