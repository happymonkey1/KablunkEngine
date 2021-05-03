#include <kablunk.h>
#include <Kablunk/Core/EntryPoint.h>

#include "Sandbox2D.h"
#include "FallingSand/FallingSand.h"



//#include "imgui.h"
class ExampleLayer : public Kablunk::Layer {
public:
	ExampleLayer()
		: Layer("Example")
	{

		
	}

	~ExampleLayer()
	{

	}


	void OnUpdate(Kablunk::Timestep ts) override {
		
	}

	void OnImGuiRender(Kablunk::Timestep ts) override {
		
	}

	void OnEvent(Kablunk::Event& e) {
		
	}
private:
	
};


class Sandbox : public Kablunk::Application {
public:
	Sandbox() {
		//PushLayer(new ExampleLayer());
		PushLayer(new Sandbox2D());
		//PushLayer(new FallingSand());
	}

	~Sandbox() {

	}

private:
	
};

Kablunk::Application* Kablunk::CreateApplication() {
	return new Sandbox();
}