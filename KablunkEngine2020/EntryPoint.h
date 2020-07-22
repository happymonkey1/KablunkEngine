#pragma once

#ifdef KABLUNK_PLATFORM_WINDOWS


extern kablunk::Application* kablunk::CreateApplication();


int main(int argc, char** argv) {
	kablunk::Log::Init();
	kablunk::Log::GetCoreLogger()->warn("test");

	KABLUNK_CORE_INFO("TEST");

	kablunk::Application* app = kablunk::CreateApplication();
	app->Run();

	delete app;
}

#endif