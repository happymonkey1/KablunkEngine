#include "kablunkpch.h"
#include "Kablunk/Utilities/PlatformUtils.h"
#include "Kablunk/Core/Application.h"

#include <commdlg.h>
#include <GLFW/glfw3.h>

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>


#include <stdio.h>
#include <Iphlpapi.h>
#include <Assert.h>
#pragma comment(lib, "iphlpapi.lib")

namespace Kablunk
{
	std::string FileDialog::OpenFile(const char* filter)
	{
		// I don't really understand this code, just look for docs on windows openfiledialog

		OPENFILENAMEA ofn; 
		CHAR sz_file[260] = { 0 }; 
		CHAR current_dir[256] = { 0 };
		ZeroMemory(&ofn, sizeof(OPENFILENAME));

		ofn.lStructSize = sizeof(OPENFILENAME);
		ofn.hwndOwner = glfwGetWin32Window((GLFWwindow*)Application::Get().GetWindow().GetNativeWindow());
		ofn.lpstrFile = sz_file;
		ofn.nMaxFile = sizeof(sz_file);
		if (GetCurrentDirectoryA(256, current_dir))
			ofn.lpstrInitialDir = current_dir;
		ofn.lpstrFilter = filter;
		ofn.nFilterIndex = 1;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

		if (GetOpenFileNameA(&ofn) == TRUE)
		{
			return ofn.lpstrFile;
		}

		return { };
	}

	std::string FileDialog::SaveFile(const char* filter)
	{
		// I don't really understand this code, just look for docs on windows openfiledialog

		OPENFILENAMEA ofn;
		CHAR sz_file[260] = { 0 };
		CHAR current_dir[256] = { 0 };
		ZeroMemory(&ofn, sizeof(OPENFILENAMEA));

		ofn.lStructSize = sizeof(OPENFILENAMEA);
		ofn.hwndOwner = glfwGetWin32Window((GLFWwindow*)Application::Get().GetWindow().GetNativeWindow());
		ofn.lpstrFile = sz_file;
		ofn.nMaxFile = sizeof(sz_file);
		if (GetCurrentDirectoryA(256, current_dir))
			ofn.lpstrInitialDir = current_dir;
		ofn.lpstrFilter = filter;
		ofn.nFilterIndex = 1;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR;

		ofn.lpstrDefExt = strchr(filter, '\0') + 1;

		if (GetSaveFileNameA(&ofn) == TRUE)
			return ofn.lpstrFile;
		

		return { };
	}


	// Adapted from https://stackoverflow.com/questions/13646621/how-to-get-mac-address-in-windows-with-c
	uint64_t MacAddress::Get()
	{
		PIP_ADAPTER_INFO AdapterInfo;
		DWORD dwBufLen = sizeof(IP_ADAPTER_INFO);

		AdapterInfo = (IP_ADAPTER_INFO*)malloc(sizeof(IP_ADAPTER_INFO));
		if (AdapterInfo == NULL) {
			KB_CORE_ERROR("Failed to find mac address");
			return 0; 
		}

		// Make an initial call to GetAdaptersInfo to get the necessary size into the dwBufLen variable
		if (GetAdaptersInfo(AdapterInfo, &dwBufLen) == ERROR_BUFFER_OVERFLOW) {
			free(AdapterInfo);
			AdapterInfo = (IP_ADAPTER_INFO*)malloc(dwBufLen);
			if (AdapterInfo == NULL) {
				KB_CORE_ERROR("MacAddress buffer overflow!");
				return 0;
			}
		}

		uint64_t mac_address{ 0 };
		if (GetAdaptersInfo(AdapterInfo, &dwBufLen) == NO_ERROR) {
			// Contains pointer to current adapter info
			PIP_ADAPTER_INFO pAdapterInfo = AdapterInfo;
			
			for (int i = 0; i < 6; ++i)
			{
				uint64_t part = static_cast<uint64_t>(pAdapterInfo->Address[i]) << (48 - i * 8);
				mac_address |= part;
			}
			
		}
		free(AdapterInfo);

		return mac_address;
	}
}
