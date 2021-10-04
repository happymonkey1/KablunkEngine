#include "kablunkpch.h"

#include "Kablunk/Core/Core.h"
#include "Kablunk/Core/Log.h"
#include "Kablunk/Scripts/NativeScriptEngine.h"

namespace Kablunk
{

	void InitCore()
	{
		NativeScriptEngine::Init();

		Log::Init();

		KB_CORE_INFO("Core initilized");
	}

	void ShutdownCore()
	{


		KB_CORE_INFO("Core shutting down!");
	}
}
