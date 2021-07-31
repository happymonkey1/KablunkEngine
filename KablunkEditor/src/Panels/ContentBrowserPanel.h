#ifndef KABLUNKEDITOR_PANELS_ASSET_PANEL_H
#define KABLUNKEDITOR_PANELS_ASSET_PANEL_H

#include <Kablunk.h>

#include <filesystem>

namespace Kablunk
{
	class ContentBrowserPanel 
	{
	public:
		ContentBrowserPanel();
		~ContentBrowserPanel() = default;

		void OnImGuiRender();

	private:
		std::filesystem::path m_current_directory;

		Asset<Texture2D> m_directory_icon;
		Asset<Texture2D> m_file_icon;
	};
}

#endif
