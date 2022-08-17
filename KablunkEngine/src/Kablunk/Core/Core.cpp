#include "kablunkpch.h"

#include "Kablunk/Core/Core.h"
#include "Kablunk/Core/Logger.h"

namespace Kablunk
{

	void InitCore()
	{
		Logger::get().init();

		KB_CORE_INFO("Core initilized");
	}

	void ShutdownCore()
	{
		KB_CORE_INFO("Core shutting down!");

		Logger::get().shutdown();
	}
}
