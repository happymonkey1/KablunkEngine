#pragma once

#include <windows.h>

#define KB_API extern "C" __declspec(dllexport)

KB_API BOOL DllMain(HANDLE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved);