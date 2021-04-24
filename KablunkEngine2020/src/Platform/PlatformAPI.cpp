#include "kablunkpch.h"
#include "PlatformAPI.h"

#include "Platform/OpenGL/OpenGLTimer.h"
#include "Platform/Windows/WindowsTimer.h"

namespace Kablunk
{

	float PlatformAPI::GetTime()
	{
		switch (PlatformAPI::GetPlatform())
		{
		case PlatformAPI::PLATFORM::Windows:    return OpenGLTimer().GetTime();
		case PlatformAPI::PLATFORM::Mac:        KB_CORE_ASSERT(false, "PlatformAPI::Mac is not currently supported!"); break;
		case PlatformAPI::PLATFORM::Linux:      KB_CORE_ASSERT(false, "PlatformAPI::Linux is not currently supported!"); break;
		case PlatformAPI::PLATFORM::Android:    KB_CORE_ASSERT(false, "PlatformAPI::Android is not currently supported!"); break;
		case PlatformAPI::PLATFORM::iOS:        KB_CORE_ASSERT(false, "PlatformAPI::iOS is not currently supported!"); break;
		case PlatformAPI::PLATFORM::None:       KB_CORE_ASSERT(false, "PlatformAPI is not set!"); break;
		}
		
		KB_CORE_ASSERT(false, "PlatformAPI timer not found!");
		return 0.0f;
	}


	PlatformAPI::PLATFORM PlatformAPI::s_Platform = PlatformAPI::PLATFORM::Windows;
}