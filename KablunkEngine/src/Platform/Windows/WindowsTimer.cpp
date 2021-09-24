#include "kablunkpch.h"
#include "Platform/Windows/WindowsTimer.h"

#include "Kablunk/Core/Log.h"

namespace Kablunk 
{
	
	WindowsTimer::WindowsTimer()
	{

	}

	float WindowsTimer::GetTime() const
	{
		KB_CORE_ASSERT(false, "Need to implement windows timer!");
		return 0.0f;
	}

}
