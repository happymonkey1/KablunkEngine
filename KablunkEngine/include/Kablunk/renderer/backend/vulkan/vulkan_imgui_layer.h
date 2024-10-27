#pragma once
#ifndef KABLUNK_RENDERER_BACKEND_VULKAN_IMGUI_LAYER_H
#define KABLUNK_RENDERER_BACKEND_VULKAN_IMGUI_LAYER_H

#include "Kablunk/Imgui/ImGuiLayer.h"
#include "Kablunk/Renderer/backend/render_command_buffer.h"

namespace kb::render::backend::vk
{ // start namespace kb::render::backend::vk

class vulkan_imgui_layer : public ImGuiLayer
{
public:
	vulkan_imgui_layer();
	vulkan_imgui_layer(const std::string& name);
	~vulkan_imgui_layer();

	virtual void OnAttach() override;
	virtual void OnDetach() override;
	virtual void OnUpdate(Timestep ts) override;
	virtual void OnImGuiRender(Timestep ts) override;
	virtual void OnEvent(Event& event) override;

	virtual void Begin() override;
	virtual void End() override;
private:
	arc<render_command_buffer> m_render_command_buffer;
};

} // end namespace kb::render::backend::vk

#endif
