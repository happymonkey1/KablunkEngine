#pragma once

#ifdef KABLUNK_PLATFORM_WINDOWS


extern kablunk::Application* kablunk::CreateApplication();


int main(int argc, char** argv) {

	KABLUNK_CORE_INFO("Initialized Kablunk Engine");
	KABLUNK_CORE_WARN("TESTING WARNING SYSTEM");
	KABLUNK_CLIENT_INFO("CLIENT LOG TEST");
	kablunk::Application* app = kablunk::CreateApplication();
	app->Run();

	delete app;
}

#endif