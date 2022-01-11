#include "kablunkpch.h"

#include "Kablunk/Imgui/ImGuiWrappers.h"


namespace Kablunk::UI
{
	void Image(const IntrusiveRef<Texture2D>& image, const ImVec2& size, const ImVec2& uv0, const ImVec2& uv1, const ImVec4& tint_col, const ImVec4& border_col)
	{
		ImGui::Image((ImTextureID)image->GetRendererID(), size, uv0, uv1, tint_col, border_col);
		ImGui::NextColumn();
		ImGui::NextColumn();
	}

	bool ImageButton(const IntrusiveRef<Texture2D>& image, const ImVec2& size, const ImVec2& uv0, const ImVec2& uv1, int frame_padding, const ImVec4& bg_col, const ImVec4& tint_col)
	{
		bool pressed = ImGui::ImageButton((ImTextureID)image->GetRendererID(), size, uv0, uv1, frame_padding, bg_col, tint_col);
		ImGui::NextColumn();
		ImGui::NextColumn();
		return pressed;
	}
}
