#include "kablunkpch.h"

#include "Kablunk/Core/Core.h"
#include "Kablunk/Core/Logger.h"

namespace Kablunk
{

	void InitCore()
	{
		auto logger = Singleton<Logger>::get();
		Singleton<Logger>::init();

		KB_CORE_INFO("Core initilized");
	}

	void ShutdownCore()
	{
		KB_CORE_INFO("Core shutting down!");

		Singleton<Logger>::shutdown();
	}
}
