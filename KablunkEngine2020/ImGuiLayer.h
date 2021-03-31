#pragma once

#include "Layer.h"
#include "MouseEvent.h"
#include "KeyEvent.h"
#include "ApplicationEvent.h"



namespace kablunk {
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

