#include <Kablunk.h>
#include <Kablunk/Core/EntryPoint.h>

#include "EditorLayer.h"

namespace kb
{

	kb::Application* kb::CreateApplication(int argc, char** argv)
	{
		ApplicationSpecification specification = {
			"Kablunk Editor",					// window name
			1920,								// width
			1080,								// height
			false,								// fullscreen
			false,								// vsync
			"",									// working directory (unused)
			true,
			true,
			threading_policy_t::single_threaded // threading policy for engine
		};

		Application& app = Singleton<Application>::get();
		app.set_application_specification(specification);
		app.init();
		app.PushLayer(new EditorLayer{});
		
		return &app;
	}

}
