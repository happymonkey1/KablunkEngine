#pragma once
#ifndef KABLUNK_EDITOR_PANELS_ASSET_EDITOR_PANEL_H
#define KABLUNK_EDITOR_PANELS_ASSET_EDITOR_PANEL_H

#include <Kablunk.h>

namespace Kablunk
{

	class AssetEditor
	{
	protected:
		AssetEditor(const char* id);

	public:
		virtual ~AssetEditor() = default;

		virtual void on_update(Timestep ts) {}
		virtual void on_event(Event& e) {}
		virtual void on_imgui_render();
		
		virtual void set_scene_context(const ref<Scene>& context) {}

		virtual void set_asset(const ref<asset::IAsset>& asset) = 0;

		bool& get_open() { return m_open; }
		bool get_open() const { return m_open; }
		void set_open(bool value);

		void set_title(const std::string& new_title);

	protected:
		void set_min_size(u32 width, u32 height);
		void set_max_size(u32 width, u32 height);

		virtual ImGuiWindowFlags get_window_flags() { return 0; }

		// optional overrides to customize window styling
		virtual void on_window_style_push() {}
		virtual void on_window_style_pop() {}
	private:
		virtual void on_open() = 0;
		virtual void on_close() = 0;
		virtual void render() = 0;

	protected:
		// cache of the title + id
		std::string m_title_with_id;
		// min and max size for the panel
		ImVec2 m_min_size, m_max_size;
		// whether the panel is open
		bool m_open = false;

	private:
		// unique identifier for the window
		std::string m_id;
	};

	class AssetEditorPanel : public RefCounted
	{
	public:
		void register_default_editors();
		void unregister_all_editors();

		void on_update(Timestep ts);
		void on_event(Event& e);
		void on_imgui_render();
		void open_editor(const ref<asset::IAsset>& asset);

		template <typename T>
		void register_editor(asset::AssetType asset_type)
		{
			static_assert(std::is_base_of<AssetEditor, T>::value, "AssetEditorPanel requires type to be derived from AssetEditor!");
			KB_CORE_ASSERT(m_registered_editors.find(asset_type) == m_registered_editors.end(), "asset editor for that type already registered!");
			m_registered_editors[asset_type] = CreateScope<T>();
		}
	private:
		// current scene context
		ref<Scene> m_scene_context = nullptr;
		// registered asset editors
		map<asset::AssetType, Scope<AssetEditor>> m_registered_editors;
	};

}

#endif
