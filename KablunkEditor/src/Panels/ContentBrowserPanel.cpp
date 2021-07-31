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
		m_directory_icon = Asset<Texture2D>("resources/content_browser/icons/directoryicon.png");
		m_file_icon = Asset<Texture2D>("resources/content_browser/icons/textfileicon.png");
	}

	void ContentBrowserPanel::OnImGuiRender()
	{
		ImGui::Begin("Content Browser");

		if (m_current_directory != s_asset_path)
		{
			if (ImGui::Button("<<<"))
				m_current_directory = m_current_directory.parent_path();
			
		}

		static float padding = 16.0f;
		static float thumbnail_size = 128.0f;
		float cell_size = thumbnail_size + padding;

		auto panel_width = ImGui::GetContentRegionAvailWidth();
		auto column_count = static_cast<int>(panel_width / cell_size);
		column_count = column_count < 1 ? 1 : column_count;

		ImGui::Columns(column_count, 0, false);
		
		auto mouse_double_click = ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left);
		// #TODO refactor so we don't iterate through the file system every frame
		for (auto& directory_entry : std::filesystem::directory_iterator{ m_current_directory })
		{
			const auto& path = directory_entry.path();
			auto relative_path = std::filesystem::relative(path, s_asset_path);
			auto filename_string = relative_path.filename().string();
			auto is_dir = directory_entry.is_directory();

			// #TODO get more file icons and adjust icon texture accordingly
			auto icon_renderer_id = is_dir ? m_directory_icon.Get()->GetRendererID() : m_file_icon.Get()->GetRendererID();
			ImGui::PushStyleColor(ImGuiCol_Button, { 0, 0, 0, 0 });
			ImGui::ImageButton((ImTextureID)icon_renderer_id, { thumbnail_size, thumbnail_size }, { 0, 1 }, { 1, 0 });
			ImGui::PopStyleColor();

			if (ImGui::BeginDragDropSource())
			{
				const auto item_path = relative_path.c_str();
				ImGui::SetDragDropPayload("CONTENT_BROWSER_ITEM", item_path, wcslen(item_path))
				ImGui::EndDragDropSource();
			}

			if (ImGui::IsItemHovered() && mouse_double_click)
			{
				if (directory_entry.is_directory())
					m_current_directory /= path.filename();
			}
			ImGui::TextWrapped(filename_string.c_str());

			ImGui::NextColumn();
		}

		ImGui::Columns(1);

		ImGui::NewLine();

		ImGui::SliderFloat("Thumbnail size", &thumbnail_size, 16, 512);
		ImGui::SliderFloat("Padding", &padding, 0, 32);

		ImGui::End();
	}
}
