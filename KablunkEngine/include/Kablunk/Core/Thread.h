#pragma once

#include <string>
#include <thread>

namespace kb
{ // start namespace Kablunk

// class that is implemented on a platform specific basis to avoid user thread library overhead
class thread
{
public:
	// default constructor
	thread() = default;
	// overloaded constructor which sets thread's name and creates a native thread
	thread(const std::string& p_name);

	template <typename func_t, typename... args_t>
	void dispatch(func_t&& func, args_t&&... args)
	{
		m_thread = std::thread(func, std::forward<args_t>(args)...);
	}

	void set_name(const std::string& name);

	void join();
private:
	// name of the thread
	std::string m_name;
	// std::thread which is used to access native threading
	std::thread m_thread;
};

class thread_signal
{
public:
	thread_signal(const std::string& p_name, bool p_manual_reset = false);

	void wait();
	void signal();
	void reset();

private:
	void* m_signal_handle = nullptr;
};

} // end namespace Kablunk
