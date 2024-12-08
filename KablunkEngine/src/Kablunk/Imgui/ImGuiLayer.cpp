#include "kablunkpch.h"
#include "Kablunk/Imgui/ImGuiLayer.h"

#include "imgui.h"

#include "Kablunk/renderer/Renderer.h"
#include "Kablunk/renderer/backend/vulkan/vulkan_imgui_layer.h"
#include "Kablunk/renderer/backend/backend_type.h"

namespace kb 
{ // start namespace kb
void ImGuiLayer::SetDarkTheme()
{
	ImGuiStyle& style = ImGui::GetStyle();
	style.Colors[ImGuiCol_WindowBg]				= { 0.133f, 0.133f, 0.133f, 1.0f };	// #222222

	style.Colors[ImGuiCol_Header]				= { 0.200f, 0.205f, 0.210f, 1.0f };
	style.Colors[ImGuiCol_HeaderHovered]		= { 0.300f, 0.305f, 0.310f, 1.0f };
	style.Colors[ImGuiCol_HeaderActive]			= { 0.15f, 0.1505f, 0.151f, 1.0f };

	style.Colors[ImGuiCol_TitleBg]				= { 0.231f, 0.231f, 0.231f, 1.0f };	// #3B3B3B
	style.Colors[ImGuiCol_TitleBgActive]		= { 0.381f, 0.381f, 0.381f, 1.0f };	// #515151
	style.Colors[ImGuiCol_TitleBgCollapsed]		= { 0.15f, 0.1505f, 0.151f, 1.0f };

	style.Colors[ImGuiCol_Button]				= { 0.494f, 0.494f, 0.494f, 1.0f };
	style.Colors[ImGuiCol_ButtonHovered]		= { 0.300f, 0.305f, 0.310f, 1.0f };
	style.Colors[ImGuiCol_ButtonActive]			= { 0.15f, 0.1505f, 0.151f, 1.0f };

	style.Colors[ImGuiCol_FrameBg]				= { 0.494f, 0.494f, 0.494f, 1.0f };	// #7E7E7E
	style.Colors[ImGuiCol_FrameBgHovered]		= { 0.300f, 0.305f, 0.310f, 1.0f };
	style.Colors[ImGuiCol_FrameBgActive]		= { 0.15f, 0.1505f, 0.151f, 1.0f };

	style.Colors[ImGuiCol_Tab]					= { 0.15f, 0.1505f, 0.151f, 1.0f };
	style.Colors[ImGuiCol_TabHovered]			= { 0.38f, 0.3805f, 0.381f, 1.0f };
	style.Colors[ImGuiCol_TabActive]			= { 0.28f, 0.2805f, 0.281f, 1.0f };
	style.Colors[ImGuiCol_TabUnfocused]			= { 0.15f, 0.1505f, 0.151f, 1.0f };
	style.Colors[ImGuiCol_TabUnfocusedActive]	= { 0.200f, 0.205f, 0.210f, 1.0f };
}

ImGuiLayer* ImGuiLayer::Create()
{
    switch (const auto backend = Singleton<render::Renderer>::get().get_render_backend_type())
    {
    case render::backend::render_backend_type_t::vulkan:
        return new render::backend::vk::vulkan_imgui_layer{};
    case render::backend::render_backend_type_t::none:
        return nullptr;
    default:
    {
        KB_CORE_ASSERT(false, "Unhandled render backend type!"); return nullptr;
    }
    }
}
} // end namespace kb
