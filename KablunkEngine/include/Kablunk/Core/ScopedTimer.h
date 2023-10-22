#pragma once
#ifndef KABLUNK_CORE_SCOPED_TIMER_H
#define KABLUNK_CORE_SCOPED_TIMER_H

#include <chrono>
#include <functional>

namespace kb
{

// adapted from https://gist.github.com/justgord/4482447
template <typename _timer_resolution_t = std::chrono::microseconds>
class scoped_timer
{
public:
	using timer_resolution_t = _timer_resolution_t;
	using callback_func_t = void(timer_resolution_t);
public:
	// default constructor
	scoped_timer()
		: m_initial_time{ std::chrono::high_resolution_clock::now() }, m_callback{ nullptr }
	{ }

	// constructor that takes a callback function
	scoped_timer(std::function<callback_func_t> callback)
		: m_initial_time{ std::chrono::high_resolution_clock::now() }, m_callback{ callback }
	{ }

	// destructor
	~scoped_timer()
	{
		// calculate time from construction to destruction
		// this is *not* perfectly accurate do to stack creation and destruction surrounding the timer class itself
		auto end_time = std::chrono::high_resolution_clock::now();
		auto nano_seconds = std::chrono::duration_cast<timer_resolution_t>(end_time - m_initial_time);

		// call callback if available 
		if (m_callback)
			m_callback(nano_seconds);
	}

private:
	// starting time point, set during constructor
	std::chrono::high_resolution_clock::time_point m_initial_time{};
	// callback for when the timer goes out of scope
	std::function<callback_func_t> m_callback = nullptr;
};

}

#endif
