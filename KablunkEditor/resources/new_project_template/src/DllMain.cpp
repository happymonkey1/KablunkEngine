#include "DllMain.h"

#include "Kablunk/Core/Log.h"
#include "Kablunk/Scripts/NativeScriptModule.h"
#include "Kablunk/Scripts/NativeScriptEngine.h"

#include <memory.h>

void DebugPrint()
{
	KB_CLIENT_INFO("Printing from DLL!");
}

BOOL APIENTRY DllMain(HANDLE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	Kablunk::SharedMemoryModule m_shared_mem = Kablunk::SharedMemoryModule{ "TEST", sizeof(spdlog::logger) };
	Kablunk::Log::DLLSetClientLogger(m_shared_mem.GetFromMemory<spdlog::logger>());


	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		// Initialize once for each new process.
		// Return FALSE to fail DLL load.
		DebugPrint();
		break;

	case DLL_THREAD_ATTACH:
		// Do thread-specific initialization.
		break;

	case DLL_THREAD_DETACH:
		// Do thread-specific cleanup.
		break;

	case DLL_PROCESS_DETACH:
		break;
	}

	return TRUE;
}