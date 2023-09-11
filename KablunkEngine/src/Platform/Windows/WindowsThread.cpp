#include "kablunkpch.h"
#include "Kablunk/Core/Thread.h"
#include "Kablunk/Core/KablunkAPI.h"

#ifdef KB_PLATFORM_WINDOWS
#	include <Windows.h>
#else
#	error "trying to include windows specific platform on non-windows platform?"
#endif

namespace kb
{ // start namespace Kablunk

// =====================
// thread implementation
// =====================

thread::thread(const std::string& p_name)
	: m_name{ p_name }
{
}

void thread::set_name(const std::string& name)
{
	HANDLE thread_handle = m_thread.native_handle();
	KB_CORE_ASSERT(thread_handle, "invalid thread handle?");

	std::wstring wide_name{ name.begin(), name.end() };
	SetThreadDescription(thread_handle, wide_name.c_str());
	SetThreadAffinityMask(thread_handle, 8);
}

void thread::join()
{
	m_thread.join();
}

// =====================

// ============================
// thread_signal implementation
// ============================

thread_signal::thread_signal(const std::string& p_name, bool p_manual_reset /*= false*/)
{
	std::wstring wide_name{ p_name.begin(), p_name.end() };
	m_signal_handle = CreateEvent(NULL, (BOOL)p_manual_reset, FALSE, wide_name.c_str());
}

void thread_signal::wait()
{
	WaitForSingleObject(m_signal_handle, INFINITE);
}

void thread_signal::signal()
{
	SetEvent(m_signal_handle);
}

void thread_signal::reset()
{
	ResetEvent(m_signal_handle);
}

// ============================

} // end namespace Kablunk
