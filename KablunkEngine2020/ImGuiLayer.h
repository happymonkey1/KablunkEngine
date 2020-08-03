#pragma once

#include "Layer.h"
#include "MouseEvent.h"
namespace kablunk {
	class KABLUNK_API ImGuiLayer : public Layer
	{
	public:
		ImGuiLayer();
		~ImGuiLayer();

		void OnAttach();
		void OnDetach();
		void OnUpdate();
		void OnEvent(Event& event);

		bool OnMousePress(MouseButtonPressedEvent& e);
	private:
		float m_Time;
	};
}

