#pragma once

#ifdef KB_PLATFORM_WINDOWS


extern Kablunk::Application* Kablunk::CreateApplication();


int main(int argc, char** argv) {
	Kablunk::Log::Init();
	KB_CORE_WARN("test");

	Kablunk::Application* app = Kablunk::CreateApplication();
	app->Run();

	delete app;
}

#endif