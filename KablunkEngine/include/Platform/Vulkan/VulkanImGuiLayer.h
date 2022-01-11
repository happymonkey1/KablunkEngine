#pragma once
#ifndef KABLUNK_PLATFORM_VULKAN_IMGUI_LAYER_H
#define KABLUNK_PLATFORM_VULKAN_IMGUI_LAYER_H

#include "Kablunk/Imgui/ImGuiLayer.h"
#include "Kablunk/Renderer/RenderCommandBuffer.h"

namespace Kablunk
{
	class VulkanImGuiLayer : public ImGuiLayer
	{
	public:
		VulkanImGuiLayer();
		VulkanImGuiLayer(const std::string& name);
		~VulkanImGuiLayer();

		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnUpdate(Timestep ts) override;
		virtual void OnImGuiRender(Timestep ts) override;
		virtual void OnEvent(Event& event) override;

		virtual void Begin() override;
		virtual void End() override;
	private:
		IntrusiveRef<RenderCommandBuffer> m_render_command_buffer;
	};
}

#endif
