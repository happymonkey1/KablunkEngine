#pragma once
#include "Core.h"
namespace kablunk {

	class KABLUNK_API Application
	{
	public:
		Application();
		virtual ~Application();

		void Run();
	};

	Application* CreateApplication();
}

