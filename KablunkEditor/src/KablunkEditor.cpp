#include <Kablunk.h>
#include <Kablunk/Core/EntryPoint.h>

#include "EditorLayer.h"

namespace Kablunk
{

	Kablunk::Application* Kablunk::CreateApplication(int argc, char** argv)
	{
		ApplicationSpecification specification = {
			"Kablunk Editor",	// window name
			1920,				// width
			1080,				// height
			true,				// fullscreen
			false,				// vsync
			"",					// working directory (unused)
			true,
			true
		};

		Application& app = Singleton<Application>::get();
		app.set_application_specification(specification);
		app.init();
		app.PushLayer(new EditorLayer{});
		
		return &app;
	}
}
