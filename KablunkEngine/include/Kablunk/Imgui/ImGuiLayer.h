#pragma once

#include "Kablunk/Core/Layer.h"
#include "Kablunk/Events/MouseEvent.h"
#include "Kablunk/Events/KeyEvent.h"
#include "Kablunk/Events/ApplicationEvent.h"



namespace Kablunk {
	class ImGuiLayer : public Layer
	{
	public:
		ImGuiLayer() : Layer{ "ImguiLayer" } {}
		virtual void Begin() = 0;
		virtual void End() = 0;

		void SetDarkTheme();

		void SetAllowEventPassing(bool allow) { m_allow_event_passing = allow;  };

		static ImGuiLayer* Create();
	private:
		bool m_allow_event_passing{ false };
	};
}

