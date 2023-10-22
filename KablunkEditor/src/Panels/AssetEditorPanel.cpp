#include "Panels/AssetEditorPanel.h"

#include "Panels/Asset/DefaultAssetViewerPanels.h"

namespace kb
{
	// ===========
	// AssetEditor
	// ===========

	AssetEditor::AssetEditor(const char* id)
		: m_id{ id }, m_min_size{ 200, 400 }, m_max_size{ 1920, 1080 }
	{
		set_title(m_id);
	}

	void AssetEditor::on_imgui_render()
	{
		if (m_open)
			return;

		bool was_open = m_open;

		on_window_style_push();
		
		ImGui::SetNextWindowSizeConstraints(m_min_size, m_max_size);
		ImGui::Begin(m_title_with_id.c_str(), &m_open, get_window_flags());

		on_window_style_pop();
		render();

		ImGui::End();

		if (was_open && !m_open)
			on_close();
	}

	void AssetEditor::set_open(bool value)
	{
		m_open = value;
		if (!m_open)
			on_close();
		else
			on_open();
	}

	void AssetEditor::set_title(const std::string& new_title)
	{
		m_title_with_id = fmt::format("{}###{}", new_title, m_id);
	}

	void AssetEditor::set_min_size(u32 width, u32 height)
	{
		if (width <= 0ul)
			width = 200ul;

		if (height <= 0ul)
			height = 400ul;

		m_min_size = ImVec2{ static_cast<f32>(width), static_cast<f32>(height) };
	}

	void AssetEditor::set_max_size(u32 width, u32 height)
	{
		if (width <= 0ul)
			width = 1920ul;

		if (height <= 0ul)
			height = 1080ul;

		if (static_cast<f32>(width) <= m_min_size.x) 
			width = static_cast<u32>(m_min_size.x * 2.f);
		if (static_cast<f32>(height) <= m_min_size.y) 
			height = static_cast<u32>(m_min_size.y * 2.f);


		m_max_size = ImVec2{ static_cast<f32>(width), static_cast<f32>(height)};
	}

	// ================



	// ================
	// AssetEditorPanel
	// ================

	void AssetEditorPanel::register_default_editors()
	{
		// register editors 
		register_editor<TextureViewer>(asset::asset_type_t::Texture);

		// warn for un-registered asset types
		for (asset::asset_type_t asset_type : asset::asset_type_iterator{})
			if (m_registered_editors.find(asset_type) == m_registered_editors.end())
				KB_CORE_WARN("[AssetEditorPanel] asset type '{}' does not have a registered editor!", asset::asset_type_to_string(asset_type));
	}

	void AssetEditorPanel::unregister_all_editors()
	{
		m_registered_editors.clear();
	}

	void AssetEditorPanel::on_update(Timestep ts)
	{
		for (auto& [_, editor] : m_registered_editors)
			editor->on_update(ts);
	}

	void AssetEditorPanel::on_event(Event& e)
	{
		for (auto& [_, editor] : m_registered_editors)
			editor->on_event(e);
	}

	void AssetEditorPanel::on_imgui_render()
	{
		for (auto& [_, editor] : m_registered_editors)
			editor->on_imgui_render();
	}

	void AssetEditorPanel::open_editor(const ref<asset::IAsset>& asset)
	{
		if (!asset)
			return;

		auto it = m_registered_editors.find(asset->get_static_type());
		if (it == m_registered_editors.end())
			return;

		box<AssetEditor>& editor = it->second;
		if (editor->get_open())
			editor->set_open(false);

		const asset::AssetMetadata& metadata = asset::try_get_asset_metadata(asset->get_id());
		KB_CORE_ASSERT(metadata.is_valid(), "asset not valid?");

		editor->set_title(metadata.filepath.filename().string());
		editor->set_asset(asset::get_asset<asset::IAsset>(asset->get_id()));
		editor->set_scene_context(m_scene_context);
		editor->set_open(true);
	}

}
