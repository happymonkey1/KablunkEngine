#include "kablunkpch.h"
#include "Kablunk/Utilities/PlatformUtils.h"
#include "Kablunk/Core/Application.h"

#include <commdlg.h>
#include <GLFW/glfw3.h>

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

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
}
