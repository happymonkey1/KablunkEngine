#include "kablunkpch.h"

#include "Kablunk/Core/Core.h"
#include "Kablunk/Core/Log.h"
#include "Kablunk/Scripts/NativeScriptEngine.h"

namespace Kablunk
{

	void InitCore()
	{
		auto logger = Singleton<Log>::get();

		KB_CORE_INFO("Core initilized");
	}

	void ShutdownCore()
	{
		KB_CORE_INFO("Core shutting down!");

		Singleton<Log>::destroy();
	}
}
