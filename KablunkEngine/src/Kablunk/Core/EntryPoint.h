#pragma once

#ifdef KB_PLATFORM_WINDOWS


extern Kablunk::Application* Kablunk::CreateApplication();


int main(int argc, char** argv) 
{
	Kablunk::Log::Init();

	KB_BEGIN_SESSION("Startup", "KablunkProfile-Startup.json");
	Kablunk::Application* app = Kablunk::CreateApplication();
	KB_END_SESSION();

	KB_BEGIN_SESSION("Runtime", "KablunkProfile-Runtime.json");
	app->Run();
	KB_END_SESSION();

	KB_BEGIN_SESSION("Shutdown", "KablunkProfile-Shutdown.json");
	delete app;
	KB_END_SESSION();
}

#endif
