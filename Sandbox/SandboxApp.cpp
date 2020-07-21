
#include <Kablunk.h>
#include <loguru.cpp>

class Sandbox : public kablunk::Application {
public:
	Sandbox() {

	}

	~Sandbox() {

	}
};

kablunk::Application* kablunk::CreateApplication() {
	return new Sandbox();
}