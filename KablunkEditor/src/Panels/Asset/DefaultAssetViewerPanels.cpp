#include "Panels/Asset/DefaultAssetViewerPanels.h"

#include <Kablunk/Imgui/ImGuiWrappers.h>

namespace kb
{
	// =============
	// TextureViewer
	// =============

	TextureViewer::TextureViewer()
		: AssetEditor{ "Texture Viewer" }
	{
		set_min_size(200, 600);
		set_max_size(500, 1000);
	}

	void TextureViewer::on_open()
	{
		if (!m_asset)
			set_open(false);
	}

	void TextureViewer::on_close()
	{
		m_asset = nullptr;
	}

	void TextureViewer::render()
	{
		f32 texture_width = static_cast<f32>(m_asset->GetWidth());
		f32 texture_height = static_cast<f32>(m_asset->GetHeight());

		float image_size = ImGui::GetWindowWidth() - 40;
		image_size = glm::min(image_size, 500.0f);

		ImGui::SetCursorPosX(20);

		UI::BeginProperties();

		UI::Image(m_asset, { image_size, image_size });

		UI::PropertyReadOnlyFloat("Width", texture_width);
		UI::PropertyReadOnlyFloat("Height", texture_width);

		UI::EndProperties();
	}

	// =============
	
}
