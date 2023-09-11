#pragma once
#ifndef KABLUNK_PLATFORM_OPENGL_IMGUI_LAYER_H
#define KABLUNK_PLATFORM_OPENGL_IMGUI_LAYER_H

#include "Kablunk/Imgui/ImGuiLayer.h"

namespace kb
{
	class OpenGLImguiLayer : public ImGuiLayer
	{
	public:
		OpenGLImguiLayer();
		virtual ~OpenGLImguiLayer();

		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnUpdate(Timestep ts) override;
		virtual void OnImGuiRender(Timestep ts) override;
		virtual void OnEvent(Event& event) override;

		virtual void Begin() override;
		virtual void End() override;
	};
}

#endif
