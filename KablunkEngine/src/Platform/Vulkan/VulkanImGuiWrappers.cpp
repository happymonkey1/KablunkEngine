#include "kablunkpch.h"

#include <vulkan/vulkan.h>

#include "Kablunk/Renderer/RendererAPI.h"
#include "Kablunk/Imgui/ImGuiWrappers.h"
#include "Platform/Vulkan/VulkanTexture.h"

#include <examples/imgui_impl_vulkan_with_textures.h>

#include <vulkan/vulkan.h>

namespace kb::UI
{
	ImTextureID GetTextureID(ref<Texture2D> texture)
	{
		ref<VulkanTexture2D> vulkan_texture = texture.As<VulkanTexture2D>();
		const VkDescriptorImageInfo& image_info = vulkan_texture->GetVulkanDescriptorInfo();
		if (!image_info.imageView)
		{
			KB_CORE_ERROR("VulkanImGuiWrapper image view is empty!");
			return 0;
		}

		return (ImTextureID)ImGui_ImplVulkan_AddTexture(image_info.sampler, image_info.imageView, image_info.imageLayout);
	}

	void Image(const ref<Image2D>& image, const ImVec2& size, const ImVec2& uv0, const ImVec2& uv1, const ImVec4& tint_col, const ImVec4& border_col)
	{
		const VulkanImageInfo& vulkan_image_info = image.As<VulkanImage2D>()->GetImageInfo();
		auto image_info = image.As<VulkanImage2D>()->GetImageInfo();

		if (!image_info.image_view)
			return;

		const auto texture_id = ImGui_ImplVulkan_AddTexture(vulkan_image_info.sampler, image_info.image_view, image.As<VulkanImage2D>()->GetDescriptor().imageLayout);
		ImGui::Image(texture_id, size, uv0, uv1, tint_col, border_col);
	}

	void Image(const ref<Texture2D>& texture, const ImVec2& size, const ImVec2& uv0, const ImVec2& uv1, const ImVec4& tint_col, const ImVec4& border_col)
	{
		ref<VulkanTexture2D> vulkan_texture = texture.As<VulkanTexture2D>();
		const VkDescriptorImageInfo& image_info = vulkan_texture->GetVulkanDescriptorInfo();
		if (!image_info.imageView)
			return;

		const auto texture_id = ImGui_ImplVulkan_AddTexture(image_info.sampler, image_info.imageView, image_info.imageLayout);
		ImGui::Image(texture_id, size, uv0, uv1, tint_col, border_col);
	}

	bool ImageButton(const ref<Texture2D>& texture, const ImVec2& size, const ImVec2& uv0, const ImVec2& uv1, int frame_padding, const ImVec4& bg_col, const ImVec4& tint_col)
	{
		ref<VulkanTexture2D> vulkan_texture = texture.As<VulkanTexture2D>();
		const VkDescriptorImageInfo& image_info = vulkan_texture->GetVulkanDescriptorInfo();
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
