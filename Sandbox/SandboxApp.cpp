#include <kablunkpch.h>
#include <Kablunk.h>

#include "imgui.h"

//#include "imgui.h"
class ExampleLayer : public kablunk::Layer {
public:
	ExampleLayer() { }

	void OnUpdate() override {
		//KB_CLIENT_INFO("ExampleLayer::Update");

		if (kablunk::Input::IsKeyPressed(KB_KEY_TAB))
			KB_CLIENT_INFO("Tab key is pressed");
	}

	virtual void OnImGuiRender() override {
		/*ImGui::Begin("Test");
		ImGui::TextUnformatted("Hello World");
		ImGui::End();*/
	}

	void OnEvent(kablunk::Event& e) {
		//KABLUNK_CLIENT_TRACE("{0}", e);

		if (e.GetEventType() == kablunk::EventType::KeyPressed) {
			kablunk::KeyPressedEvent& keyEvent = (kablunk::KeyPressedEvent&)e;

			if (keyEvent.GetKeyCode() == KB_KEY_TAB)
				KB_CLIENT_TRACE("Tab key pressed");
		}
	}
};


class Sandbox : public kablunk::Application {
public:
	Sandbox() {
		PushLayer(new ExampleLayer());
	}

	~Sandbox() {

	}
};

kablunk::Application* kablunk::CreateApplication() {
	return new Sandbox();
}