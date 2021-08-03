#ifndef KABLUNKEDITOR_PANELS_ASSET_PANEL_H
#define KABLUNKEDITOR_PANELS_ASSET_PANEL_H

#include <Kablunk.h>

#include <filesystem>
#include <shared_mutex>

namespace Kablunk
{
	class ContentBrowserPanel 
	{
	public:
		ContentBrowserPanel();
		~ContentBrowserPanel() = default;

		void OnImGuiRender();
		void OnUpdate(Timestep ts);


	private:
		void UpdateDirectoryList();
		std::filesystem::path m_current_directory;
		std::vector<std::filesystem::directory_entry> m_directory_entries{};
		mutable std::shared_mutex m_mutex;

		Asset<Texture2D> m_directory_icon;
		Asset<Texture2D> m_file_icon;

		float m_update_directory_timer_max{ 1.0f };
		float m_update_directory_timer{ m_update_directory_timer_max };
	};
}

#endif
