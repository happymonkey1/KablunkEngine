#include "kablunkpch.h"

#include "Kablunk/Renderer/RendererAPI.h"
#include "Kablunk/Imgui/ImGuiWrappers.h"
#include "Platform/Vulkan/VulkanTexture.h"

#include <backends/imgui_impl_vulkan.h>

namespace Kablunk::UI
{
	void Image(const IntrusiveRef<Texture2D>& texture, const ImVec2& size, const ImVec2& uv0, const ImVec2& uv1, const ImVec4& tint_col, const ImVec4& border_col)
	{
		IntrusiveRef<VulkanTexture2D> vulkanTexture = texture.As<VulkanTexture2D>();
		const VkDescriptorImageInfo& imageInfo = vulkanTexture->GetVulkanDescriptorInfo();
		if (!imageInfo.imageView)
			return;
		const auto textureID = 0;// ImGui_ImplVulkan_AddTexture(imageInfo.sampler, imageInfo.imageView, imageInfo.imageLayout);
		ImGui::Image(textureID, size, uv0, uv1, tint_col, border_col);
		ImGui::NextColumn();
		ImGui::NextColumn();
	}

	bool ImageButton(const IntrusiveRef<Texture2D>& texture, const ImVec2& size, const ImVec2& uv0, const ImVec2& uv1, int frame_padding, const ImVec4& bg_col, const ImVec4& tint_col)
	{
		IntrusiveRef<VulkanTexture2D> vulkanTexture = texture.As<VulkanTexture2D>();
		const VkDescriptorImageInfo& imageInfo = vulkanTexture->GetVulkanDescriptorInfo();
		if (!imageInfo.imageView)
			return false;
		const auto texture_id = 0;// ImGui_ImplVulkan_AddTexture(imageInfo.sampler, imageInfo.imageView, imageInfo.imageLayout);
		bool pressed = ImGui::ImageButton(texture_id, size, uv0, uv1, frame_padding, bg_col, tint_col);
		ImGui::NextColumn();
		ImGui::NextColumn();
		return pressed;
	}
}
