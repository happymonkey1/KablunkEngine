#pragma once
#ifndef KABLUNK_EDITOR_PANELS_ASSET_REGISTRY_PANEL_H
#define KABLUNK_EDITOR_PANELS_ASSET_REGISTRY_PANEL_H

#include <Kablunk.h>

namespace kb
{

	class AssetRegistryPanel : public RefCounted
	{
	public:
		AssetRegistryPanel() = default;
		~AssetRegistryPanel() = default;

		void on_imgui_render();

		void set_visible(bool value) { m_open = value; }

		bool& get_visible() { return m_open; }
		bool get_visible() const { return m_open; }
	private:
		bool m_open = false;
	};

}

#endif
