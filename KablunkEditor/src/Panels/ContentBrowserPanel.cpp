#include "Panels/ContentBrowserPanel.h"
#include "Kablunk/Project/Project.h"
#include "Kablunk/Imgui/ImGuiWrappers.h"

#include <imgui/imgui.h>
#include <glm/gtc/type_ptr.hpp>

#include "imgui/imgui_internal.h"

#include <thread>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>

namespace Kablunk
{
	//extern const std::filesystem::path g_asset_path = "assets";
	//extern const std::filesystem::path g_resources_path = "resources";

	ContentBrowserPanel::ContentBrowserPanel()
		: m_current_directory{ Project::GetActive() ? Project::GetAssetDirectoryPath() : "" }
	{
		m_directory_icon = Asset<Texture2D>("resources/content_browser/icons/directoryicon.png");
		m_file_icon = Asset<Texture2D>("resources/content_browser/icons/textfileicon.png");
		m_back_button = Asset<Texture2D>("resources/content_browser/icons/back_button.png");
		m_forward_button = Asset<Texture2D>("resources/content_browser/icons/forward_button.png");
		m_refresh_button = Asset<Texture2D>("resources/content_browser/icons/refresh_button.png");

		memset(m_search_buffer, 0, sizeof(char) * MAX_SEARCH_BUFFER_LENGTH);
		Refresh();
	}

	void ContentBrowserPanel::OnImGuiRender()
	{
		

		ImGui::Begin("Content Browser", NULL, ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar);

		if (!Project::GetActive())
		{
			ImGui::End();
			return;
		}

		RenderTopBar();

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

			auto working_dir = Project::GetAssetDirectory();
			ImGui::Text(working_dir.string().c_str());

			for (auto& directory_entry : m_directory_entries)
			{
				if (directory_entry.is_directory())
				{
					const auto& path			= directory_entry.path();
					auto relative_path			= std::filesystem::relative(path, Project::GetAssetDirectoryPath());
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
			auto files_column_width = ImGui::GetContentRegionAvail().x;
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
					auto relative_path = std::filesystem::relative(path, Project::GetAssetDirectory());
					auto filename_string = relative_path.filename().string();
					auto is_dir = directory_entry.is_directory();

					ImGui::TableNextColumn();
					ImGui::PushID(i++);

					// #TODO include more file icons and adjust icon texture accordingly
					
					auto icon = is_dir ? m_directory_icon.Get() : m_file_icon.Get();
					ImGui::PushStyleColor(ImGuiCol_Button, { 0, 0, 0, 0 });
					UI::ImageButton(icon, { thumbnail_size, thumbnail_size });
					ImGui::PopStyleColor();

					if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
					{

						// drag drop selection icon + name 
						UI::Image(icon, { 20, 20 });
						ImGui::SameLine();
						ImGui::TextUnformatted(relative_path.stem().string().c_str());
						
						const auto item_path = relative_path.c_str();
						ImGui::SetDragDropPayload("CONTENT_BROWSER_ITEM", item_path, (wcslen(item_path) + 1) * sizeof(wchar_t), ImGuiCond_Once);
						ImGui::EndDragDropSource();
					}

					if (ImGui::IsItemHovered() && mouse_double_click)
					{
						if (directory_entry.is_directory())
						{
							m_current_directory /= path.filename();
							Refresh();
						}
						else if (directory_entry.path().extension() == FileExtensions::FBX)
						{
						
							// #TODO open model in asset viewer
						
						}
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
		//Threading::JobSystem::AddJob(std::function<void()>([this]() { UpdateDirectoryList(); }));
		/*m_update_directory_timer += ts.GetMiliseconds() / 1000.0f;
		if (m_update_directory_timer >= m_update_directory_timer_max)
		{
			Threading::JobSystem::AddJob(std::function<void()>([this]() { UpdateDirectoryList(); }));

			m_update_directory_timer -= m_update_directory_timer_max;
		}*/
	}

	void ContentBrowserPanel::RenderTopBar()
	{
		ImGui::BeginChild("##top_bar", { 0, 30 });
		

		if (UI::ImageButton(m_back_button.Get(), {22, 22}) && m_current_directory != Project::GetAssetDirectory())
		{
			m_current_directory = m_current_directory.parent_path();
			Refresh();
		}
		
		ImGui::SameLine();

		if (UI::ImageButton(m_forward_button.Get(), { 22, 22 }))
		{
			// #TODO go to next directory
			KB_CORE_WARN("Forward directory not implemented!");
		}

		ImGui::SameLine();

		if (UI::ImageButton(m_refresh_button.Get(), { 22, 22 }))
		{
			Refresh();
		}

		ImGui::SameLine();
		
		ImGui::PushItemWidth(200);
		if (ImGui::InputTextWithHint("##search_bar", "Search...", m_search_buffer, MAX_SEARCH_BUFFER_LENGTH))
		{
			if (strlen(m_search_buffer) == 0)
			{
				// #TODO reset search to current directory
			}
			else
			{
				// #TODO add item searching
				KB_CORE_WARN("searching for items not implemented!");
			}

			ImGui::PopItemWidth();
		}
		
		ImGui::SameLine();


		// #TODO update to use project's asset directory when projects are implemented
		auto project_asset_dir = Project::GetProjectDirectory();
		auto asset_dir_name = std::filesystem::relative(m_current_directory, project_asset_dir);
		auto text_size = ImGui::CalcTextSize(asset_dir_name.string().c_str());
		if (ImGui::Selectable(asset_dir_name.string().c_str(), false, 0, { text_size.x, 22 }))
		{
			m_current_directory = Project::GetAssetDirectoryPath();
		}

		// #TODO add other directories in current path



		ImGui::EndChild();
	}

	void ContentBrowserPanel::Refresh()
	{
		if (m_current_directory.empty())
			return;

		// #TODO probably better to store as a hashmap instead of recreating vector 
		m_directory_entries = {};
		for (auto& directory_entry : std::filesystem::directory_iterator{ m_current_directory })
			m_directory_entries.push_back(directory_entry);
	}

}
