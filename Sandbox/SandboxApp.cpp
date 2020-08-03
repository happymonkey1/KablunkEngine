#include <kablunkpch.h>
#include <Kablunk.h>

class TestLayer : public kablunk::Layer {
public:
	TestLayer() : Layer("TestLayer") { }

	void OnUpdate() override {
		KABLUNK_CLIENT_INFO("TestLayer::Update");
	}

	void OnEvent(kablunk::Event& event) {
		KABLUNK_CLIENT_TRACE("{0}", event);
	}
};


class Sandbox : public kablunk::Application {
public:
	Sandbox() {
		PushLayer(new TestLayer());
		PushOverlay(new kablunk::ImGuiLayer());
	}

	~Sandbox() {

	}
};

kablunk::Application* kablunk::CreateApplication() {
	return new Sandbox();
}