#pragma once

#include "Kablunk/Core/Layer.h"
#include "Kablunk/Events/MouseEvent.h"
#include "Kablunk/Events/KeyEvent.h"
#include "Kablunk/Events/ApplicationEvent.h"



namespace Kablunk {
	class KABLUNK_API ImGuiLayer : public Layer
	{
	public:
		ImGuiLayer();
		~ImGuiLayer();

		virtual void OnDetach() override;
		virtual void OnAttach() override;
		virtual void OnImGuiRender() override;
		void Begin();
		void End();
	private:
		float m_Time;
	};
}

