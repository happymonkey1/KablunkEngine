#include "Application.h"

#include "Event.h"
#include "ApplicationEvent.h"
#include "Log.h"
#include <loguru.cpp>




namespace kablunk {

	Application::Application() {

	}

	Application::~Application() {

	}

	void Application::Run() {
		WindowResizeEvent e(1280, 720);
		KABLUNK_CORE_INFO(e.ToString());
		while (true) {

		}
	}
}