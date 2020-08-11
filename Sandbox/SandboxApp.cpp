#include <kablunkpch.h>
#include <Kablunk.h>



class Sandbox : public kablunk::Application {
public:
	Sandbox() {
		PushOverlay(new kablunk::ImGuiLayer());
	}

	~Sandbox() {

	}
};

kablunk::Application* kablunk::CreateApplication() {
	return new Sandbox();
}