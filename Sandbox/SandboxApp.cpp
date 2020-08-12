#include <kablunkpch.h>
#include <Kablunk.h>

#include <glm/vec3.hpp> // glm::vec3
#include <glm/vec4.hpp> // glm::vec4
#include <glm/mat4x4.hpp> // glm::mat4
#include <glm/ext/matrix_transform.hpp> // glm::translate, glm::rotate, glm::scale
#include <glm/ext/matrix_clip_space.hpp> // glm::perspective
#include <glm/ext/scalar_constants.hpp> // glm::pi


class ExampleLayer : public kablunk::Layer {
public:
	ExampleLayer() { }

	void OnUpdate() override {
		/*KB_CLIENT_INFO("ExampleLayer::Update");

		if (kablunk::Input::IsKeyPressed(KB_KEY_TAB))
			KB_CLIENT_INFO("Tab key is pressed");*/
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
		PushOverlay(new kablunk::ImGuiLayer());
	}

	~Sandbox() {

	}
};

kablunk::Application* kablunk::CreateApplication() {
	return new Sandbox();
}