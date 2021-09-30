#pragma once

#include <windows.h>

#define KB_DLL_API extern "C" __declspec(dllexport)

KB_DLL_API void DebugPrint();
KB_DLL_API BOOL DllMain(HANDLE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved);