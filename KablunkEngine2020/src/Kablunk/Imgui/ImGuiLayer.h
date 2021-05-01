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
		virtual void OnImGuiRender(Timestep ts) override;
		void Begin();
		void End();
	private:
		float m_Time;
	};
}

