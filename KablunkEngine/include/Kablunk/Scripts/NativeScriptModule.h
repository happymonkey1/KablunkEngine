#ifndef KABLUNK_SCRIPTS_NATIVE_SCRIPT_MODULE_H
#define KABLUNK_SCRIPTS_NATIVE_SCRIPT_MODULE_H

#include "Kablunk/Core/Core.h"
#include <string>

#ifdef KB_PLATFORM_WINDOWS
#	include <windows.h>
#	include <stdio.h>
#else
#	error "Native scripting is only supported on windows!"
#endif

namespace Kablunk
{
	class ProcPtr
	{
	public:
		explicit ProcPtr(FARPROC ptr) : m_ptr{ ptr } {}

		template <typename T, typename = std::enable_if_t<std::is_function_v<T>>>
		operator T* () const
		{
			return reinterpret_cast<T*>(m_ptr);
		}

	private:
		FARPROC m_ptr;
	};

	class NativeScriptModule
	{
	public:
		explicit NativeScriptModule(const std::string& dll_name, const std::string& dll_path);
		~NativeScriptModule();
		
		ProcPtr operator[](const std::string& function_name)
		{
			return ProcPtr{ GetProcAddress(m_dll_handle, function_name.c_str()) };
		}

		HMODULE GetHandle() const { return m_dll_handle; }


		operator bool() const { return m_dll_handle != NULL; }

	private:
		std::string m_dll_name;
		HMODULE m_dll_handle;
	};

	class SharedMemoryModule
	{
	public:
		explicit SharedMemoryModule(const std::string& name, size_t size);
		~SharedMemoryModule();

		template <typename T>
		T* GetFromMemory()
		{
			return (T*)m_buffer;
		}

		template <typename T>
		void SetMemory(T* data, size_t size)
		{
			T* data_in_buffer = (T*)m_buffer;
			data_in_buffer = (T*)data;
		}
	private:
		std::string m_name = "";
		void* m_handle = nullptr;
		void* m_buffer = nullptr;
		size_t m_buffer_size = 0;
	};
}

#endif
