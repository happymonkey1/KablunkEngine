#pragma once

#ifdef KABLUNK_PLATFORM_WINDOWS


extern kablunk::Application* kablunk::CreateApplication();


int main(int argc, char** argv) {
	kablunk::Log::Init();
	KB_CORE_WARN("test");

	kablunk::Application* app = kablunk::CreateApplication();
	app->Run();

	delete app;
}

#endif