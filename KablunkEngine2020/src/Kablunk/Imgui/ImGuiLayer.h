#pragma once

#include "Kablunk/Core/Layer.h"
#include "Kablunk/Events/MouseEvent.h"
#include "Kablunk/Events/KeyEvent.h"
#include "Kablunk/Events/ApplicationEvent.h"



namespace Kablunk {
	class ImGuiLayer : public Layer
	{
	public:
		ImGuiLayer();
		~ImGuiLayer();

		virtual void OnDetach() override;
		virtual void OnAttach() override;
		virtual void OnEvent(Event& e) override;
		virtual void OnImGuiRender(Timestep ts) override;
		void Begin();
		void End();

		void SetAllowEventPassing(bool allow) { m_allow_event_passing = allow; }
	private:
		bool m_allow_event_passing{ true };
		float m_time;
	};
}

