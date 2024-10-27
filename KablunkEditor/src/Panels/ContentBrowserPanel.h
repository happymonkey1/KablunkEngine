#ifndef KABLUNKEDITOR_PANELS_ASSET_PANEL_H
#define KABLUNKEDITOR_PANELS_ASSET_PANEL_H

#include <Kablunk.h>

#include "Panels/AssetEditorPanel.h"

#include <filesystem>
#include <shared_mutex>

namespace kb
{
	constexpr const int MAX_SEARCH_BUFFER_LENGTH = 128;

	// #TODO move elsewhere
	struct FileExtensions
	{
		static constexpr const char* KABLUNK_SCENE  = ".kablunkscene";
		static constexpr const char* FBX			= ".fbx";
		static constexpr const char* OBJ			= ".obj";
		static constexpr const char* PNG			= ".png";
		static constexpr const char* JPEG			= ".jpeg";
		static constexpr const char* JPG			= ".jpg";
	};

	class ContentBrowserPanel 
	{
	public:
		ContentBrowserPanel(const arc<AssetEditorPanel>& asset_editor_panel);
		~ContentBrowserPanel() = default;

		void OnImGuiRender();
		void OnUpdate(Timestep ts);

		// #TODO should probably replace this, but currently exposes working directory so opening projects can set the working dir
		void SetCurrentDirectory(const std::filesystem::path& path) { m_current_directory = path; Refresh(); }

		void process_directory(const std::filesystem::path& dir_path);

	private:
		

		void RenderTopBar();
		void Refresh();
		std::filesystem::path m_current_directory;
		std::vector<std::filesystem::directory_entry> m_directory_entries{};
		mutable std::shared_mutex m_mutex;

		arc<render::backend::texture_2d> m_directory_icon;
		arc<render::backend::texture_2d> m_file_icon;
		arc<render::backend::texture_2d> m_back_button;
		arc<render::backend::texture_2d> m_forward_button;
		arc<render::backend::texture_2d> m_refresh_button;
		arc<render::backend::texture_2d> m_asset_icon;

		arc<AssetEditorPanel> m_asset_editor_panel;

		char m_search_buffer[MAX_SEARCH_BUFFER_LENGTH];

		float m_update_directory_timer_max{ 1.0f };
		float m_update_directory_timer{ m_update_directory_timer_max };
	};
}

#endif
