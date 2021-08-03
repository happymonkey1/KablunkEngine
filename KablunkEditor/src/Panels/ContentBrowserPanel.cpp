#include "Panels/ContentBrowserPanel.h"

#include <imgui/imgui.h>
#include <glm/gtc/type_ptr.hpp>

#include "imgui/imgui_internal.h"

#include <thread>

namespace Kablunk
{
	extern const std::filesystem::path g_asset_path = "assets";

	ContentBrowserPanel::ContentBrowserPanel()
		: m_current_directory{ g_asset_path }
	{
		m_directory_icon = Asset<Texture2D>("resources/content_browser/icons/directoryicon.png");
		m_file_icon = Asset<Texture2D>("resources/content_browser/icons/textfileicon.png");
	}

	void ContentBrowserPanel::OnImGuiRender()
	{
		ImGui::Begin("Content Browser");

		
		auto panel_width = ImGui::GetContentRegionAvailWidth();
		if (m_current_directory != g_asset_path)
		{
			if (ImGui::Button("<<<"))
				m_current_directory = m_current_directory.parent_path();

			// #TODO currently hardcoded, should be calculated during runtime
			ImGui::SameLine(panel_width / 2.0f + 10.0f);
		}
		else
		{
			// #TODO currently hardcoded, should be calculated during runtime
			ImGui::SetCursorPosX(panel_width / 2.0f + 10.0f);
		}
		auto current_dir_string = m_current_directory.string();
		ImGui::Text(current_dir_string.c_str());
		
		
		ImGui::Separator();	

		static float padding = 16.0f;
		static float thumbnail_size = 48.0f;
		float cell_size = thumbnail_size + padding;

		//std::shared_lock lock{ m_mutex };

		ImGuiTableFlags content_browser_panel_flags = ImGuiTableFlags_BordersInnerV;
		if (ImGui::BeginTable("Directory Browser", 2, content_browser_panel_flags))
		{
			auto imgui_table = ImGui::GetCurrentTable();
			//ImGui::TableSetupColumn("##Directories", ImGuiTableColumnFlags_WidthFixed, 75.0f);
			//ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { 4.0f, 4.0f });
			//ImGui::TableSetupColumn("##Directories", ImGuiTableColumnFlags_WidthStretch, 75.0f);
			//ImGui::TableHeadersRow();
			
			// #TODO fix width of first column. Minimum width with stretching?
			ImGui::TableNextColumn();

			ImGui::TableSetColumnIndex(0);
			ImGui::TableSetColumnWidthAutoAll(imgui_table);

			auto working_dir = g_asset_path.string();
			ImGui::Text(working_dir.c_str());

			for (auto& directory_entry : m_directory_entries)
			{
				if (directory_entry.is_directory())
				{
					const auto& path			= directory_entry.path();
					auto relative_path			= std::filesystem::relative(path, g_asset_path);
					auto relative_path_string	= relative_path.string();

					ImGuiTreeNodeFlags directory_tree_node_flags = ImGuiTreeNodeFlags_OpenOnArrow;
					// #TODO should only draw tree node if are files within directory
					if (ImGui::TreeNodeEx(relative_path_string.c_str(), directory_tree_node_flags))
					{
						// #TODO recursively populate inner directories;
						ImGui::Text("FIXME!");
						ImGui::TreePop();
					}
				}
			}


			auto cell_rect = ImGui::TableGetCellBgRect(imgui_table, 0);
			auto directory_panel_width = cell_rect.Max.x - cell_rect.Min.x;
			//ImGui::PopStyleVar(); // Frame padding
			ImGui::TableNextColumn();
			
			// Calculate how many columns we need to display files/folders
			auto files_column_width = ImGui::GetContentRegionAvailWidth();
			auto column_count = static_cast<int>(files_column_width / cell_size);
			column_count = column_count < 1 ? 1 : column_count;

			//ImGui::TableSetColumnIndex(1);
			//ImGui::TableSetColumnWidth(1, panel_width - directory_panel_width);
			int i = 0;
			if (ImGui::BeginTable("Intra Directory Browser", column_count))
			{
				
				auto mouse_double_click = ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left);
				for (auto& directory_entry : m_directory_entries)
				{
					const auto& path = directory_entry.path();
					auto relative_path = std::filesystem::relative(path, g_asset_path);
					auto filename_string = relative_path.filename().string();
					auto is_dir = directory_entry.is_directory();

					ImGui::TableNextColumn();
					ImGui::PushID(i++);

					// #TODO include more file icons and adjust icon texture accordingly
					auto icon_renderer_id = is_dir ? m_directory_icon.Get()->GetRendererID() : m_file_icon.Get()->GetRendererID();
					ImGui::PushStyleColor(ImGuiCol_Button, { 0, 0, 0, 0 });
					ImGui::ImageButton((ImTextureID)icon_renderer_id, { thumbnail_size, thumbnail_size }, { 0, 1 }, { 1, 0 });
					ImGui::PopStyleColor();

					if (ImGui::BeginDragDropSource())
					{
						const auto item_path = relative_path.c_str();
						ImGui::SetDragDropPayload("CONTENT_BROWSER_ITEM", item_path, (wcslen(item_path) + 1) * sizeof(wchar_t), ImGuiCond_Once);
						ImGui::EndDragDropSource();
					}

					if (ImGui::IsItemHovered() && mouse_double_click)
					{
						if (directory_entry.is_directory())
							m_current_directory /= path.filename();
					}
					ImGui::TextWrapped(filename_string.c_str());


					ImGui::PopID();
				}

				
				ImGui::EndTable(); // File Browser
			}

			ImGui::EndTable(); // Directory Browser
		}
		ImGui::NewLine();

		static bool debug_settings = false;
		ImGui::Checkbox("Settings", &debug_settings);
		if (debug_settings)
		{
			ImGui::SliderFloat("Thumbnail size", &thumbnail_size, 16, 512);
			ImGui::SliderFloat("Padding", &padding, 0, 32);
		}

		ImGui::End();
	}

	void ContentBrowserPanel::OnUpdate(Timestep ts)
	{
		m_update_directory_timer += ts.GetMiliseconds() / 1000.0f;
		if (m_update_directory_timer >= m_update_directory_timer_max)
		{
			//std::thread update_directory_task( &ContentBrowserPanel::UpdateDirectoryList, this );
			//update_directory_task.join();
			UpdateDirectoryList();


			m_update_directory_timer -= m_update_directory_timer_max;
		}
	}

	void ContentBrowserPanel::UpdateDirectoryList()
	{
		//std::unique_lock lock{ m_mutex };

		// #TODO probably better to store as a hashmap instead of recreating vector 
		m_directory_entries = {};
		for (auto& directory_entry : std::filesystem::directory_iterator{ m_current_directory })
			m_directory_entries.push_back(directory_entry);
	}

}
