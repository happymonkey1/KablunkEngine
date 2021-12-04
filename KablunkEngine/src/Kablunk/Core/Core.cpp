#include "kablunkpch.h"

#include "Kablunk/Core/Core.h"
#include "Kablunk/Core/Log.h"
#include "Kablunk/Scripts/NativeScriptEngine.h"

namespace Kablunk
{

	void InitCore()
	{
		Log::Init();

#if KB_NATIVE_SCRIPTING
		NativeScriptEngine::Init();
#endif

		KB_CORE_INFO("Core initilized");
	}

	void ShutdownCore()
	{
		KB_CORE_INFO("Core shutting down!");

#if KB_NATIVE_SCRIPTING
		NativeScriptEngine::Shutdown();
#endif

		Log::Shutdown();


	}
}
