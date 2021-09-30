#include "kablunkpch.h"

#include "Kablunk/Scripts/NativeScriptModule.h"

namespace Kablunk
{

	NativeScriptModule::NativeScriptModule(const std::string& dll_name, const std::string& dll_path)
		: m_dll_name{ dll_name }, m_dll_handle { NULL }
	{
		std::string full_dll_path = dll_path + '/' + dll_name;
		std::replace(full_dll_path.begin(), full_dll_path.end(), '/', '\\'); // only for windows
		m_dll_handle = LoadLibraryA(full_dll_path.c_str());
	}

	NativeScriptModule::~NativeScriptModule()
	{
		if (m_dll_handle != NULL)
		{
			bool freed = FreeLibrary(m_dll_handle);
			if (!freed)
				KB_CORE_ERROR("DLL '{0}' failed while freeing", m_dll_name);
			else
				KB_CORE_TRACE("DLL '{0}' was freed!", m_dll_name);
		}
	}

	SharedMemoryModule::SharedMemoryModule(const std::string& name, size_t size)
		: m_name{ name }, m_buffer_size{ size }
	{
		m_handle = CreateFileMappingA(
			INVALID_HANDLE_VALUE,
			NULL,
			PAGE_READWRITE,
			0,
			m_buffer_size,
			m_name.c_str()
		);

		if (m_handle == nullptr)
		{
			KB_CORE_ERROR("Failed to map shared memory!");
			return;
		}

		m_buffer = MapViewOfFile(m_handle, FILE_MAP_ALL_ACCESS, 0, 0, 0);
	}

	SharedMemoryModule::~SharedMemoryModule()
	{
		UnmapViewOfFile(m_buffer);

		CloseHandle(m_handle);
	} 
}
