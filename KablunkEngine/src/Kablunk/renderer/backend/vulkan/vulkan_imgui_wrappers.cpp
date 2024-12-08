#include "kablunkpch.h"

#include <vulkan/vulkan.h>

#include "Kablunk/Imgui/ImGuiWrappers.h"
#include "kablunk/renderer/backend/vulkan/vulkan_texture.h"

#include <examples/imgui_impl_vulkan_with_textures.h>

#include <vulkan/vulkan.h>

namespace kb::UI
{

// I'm lazy
using Texture2D = kb::render::backend::texture_2d;
using Image2D = kb::render::backend::image_2d;
using VulkanTexture2D = kb::render::backend::vk::vulkan_texture_2d;
using VulkanImage2D = kb::render::backend::vk::vulkan_image_2d;


ImTextureID GetTextureID(arc<Texture2D> texture)
{
	arc<VulkanTexture2D> vulkan_texture = texture.As<VulkanTexture2D>();
	const VkDescriptorImageInfo& image_info = vulkan_texture->get_vk_descriptor_image_info();
	if (!image_info.imageView)
	{
		KB_CORE_ERROR("VulkanImGuiWrapper image view is empty!");
		return 0;
	}

	return (ImTextureID)ImGui_ImplVulkan_AddTexture(image_info.sampler, image_info.imageView, image_info.imageLayout);
}

void Image(const arc<Image2D>& image, const ImVec2& size, const ImVec2& uv0, const ImVec2& uv1, const ImVec4& tint_col, const ImVec4& border_col)
{
	const render::backend::vk::VulkanImageInfo& vulkan_image_info = image.As<VulkanImage2D>()->get_vk_image_info();
	auto image_info = image.As<VulkanImage2D>()->get_vk_image_info();

	if (!image_info.image_view)
		return;

	const auto texture_id = ImGui_ImplVulkan_AddTexture(vulkan_image_info.sampler, image_info.image_view, image.As<VulkanImage2D>()->get_vk_image_info_descriptor().imageLayout);
	ImGui::Image(texture_id, size, uv0, uv1, tint_col, border_col);
}

void Image(const arc<Texture2D>& texture, const ImVec2& size, const ImVec2& uv0, const ImVec2& uv1, const ImVec4& tint_col, const ImVec4& border_col)
{
	arc<VulkanTexture2D> vulkan_texture = texture.As<VulkanTexture2D>();
	const VkDescriptorImageInfo& image_info = vulkan_texture->get_vk_descriptor_image_info();
	if (!image_info.imageView)
		return;

	const auto texture_id = ImGui_ImplVulkan_AddTexture(image_info.sampler, image_info.imageView, image_info.imageLayout);
	ImGui::Image(texture_id, size, uv0, uv1, tint_col, border_col);
}

bool ImageButton(const arc<Texture2D>& texture, const ImVec2& size, const ImVec2& uv0, const ImVec2& uv1, int frame_padding, const ImVec4& bg_col, const ImVec4& tint_col)
{
	arc<VulkanTexture2D> vulkan_texture = texture.As<VulkanTexture2D>();
	const VkDescriptorImageInfo& image_info = vulkan_texture->get_vk_descriptor_image_info();
	if (!image_info.imageView)
		return false;

	const auto texture_id = ImGui_ImplVulkan_AddTexture(image_info.sampler, image_info.imageView, image_info.imageLayout);
	bool pressed = ImGui::InvisibleButton("##click_button", size);

	const ImColor button_tint = IM_COL32(192, 192, 192, 255);

	float frame_padding_f = static_cast<float>(frame_padding);
	UI::DrawButtonImage(
		texture,
		button_tint,
		UI::ColorWithMultipliedValue(button_tint, 1.3f),
		UI::ColorWithMultipliedValue(button_tint, 0.8f),
		UI::RectExpanded(UI::GetItemRect(), -frame_padding_f, -frame_padding_f)
	);

	return pressed;
}
}
