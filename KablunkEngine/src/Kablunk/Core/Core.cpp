#include "kablunkpch.h"

#include "Kablunk/Core/Core.h"
#include "Kablunk/Core/Logger.h"

namespace Kablunk
{

	void InitCore()
	{
		Logger::get().init();

		KB_CORE_INFO("Logger initilized");

		memory::GeneralAllocator::get_mutable().init();

		KB_CORE_INFO("Core initialized");
	}

	void ShutdownCore()
	{
		KB_CORE_INFO("Core shutting down!");

		Logger::get().shutdown();
	}
}
