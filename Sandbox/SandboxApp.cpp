
#include <Kablunk.h>

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