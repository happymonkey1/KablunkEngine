#ifndef KABLUNK_CORE_TIMER_H
#define KABLUNK_CORE_TIMER_H

#include "Kablunk/Core/Core.h"
#include "Kablunk/Core/ScopedTimer.h"
#include <chrono>

namespace Kablunk
{ // start namespace Kablunk

// default timer used in engine (sacrifices *some* precision for performance). Use nano_timer for more exact timings
class timer
{
public:
	// typedef for underlying timer resolution
	using timer_resolution_t = std::chrono::milliseconds;
public:
	// default constructor
	KB_FORCE_INLINE timer() { reset(); }
	// default destructor
	~timer() = default;
	// reset the timer's start point to the current point in time
	KB_FORCE_INLINE void reset() { m_start_time = std::chrono::high_resolution_clock::now(); }
	// get the elapsed time between the recorded start point and current point in time
	KB_FORCE_INLINE f32 get_elapsed()
	{
		return std::chrono::duration_cast<timer_resolution_t>(
			std::chrono::high_resolution_clock::now() - m_start_time
		).count() * 0.001f * 0.001f;
	}
	// get the elapsed time (in milliseconds) between the recorded start point and current point in time
	KB_FORCE_INLINE f32 get_elapsed_ms()
	{
		return std::chrono::duration_cast<timer_resolution_t>(
			std::chrono::high_resolution_clock::now() - m_start_time
		).count() * 0.001f;
	}
private:
	std::chrono::time_point<std::chrono::high_resolution_clock> m_start_time;
};

class performance_profiler
{
public:
	performance_profiler() = default;
	~performance_profiler() = default;

	// add frame timing to a specific key
	void set_per_frame_timing(const char* name, f32 time)
	{
		std::scoped_lock<std::mutex> lock(m_per_frame_data_mutex);

		if (!m_per_frame_data_map.contains(name))
			m_per_frame_data_map[name] = 0.0f;

		m_per_frame_data_map[name] += time;
	}

	// reset recorded frame times
	void clear()
	{
		std::scoped_lock<std::mutex> lock(m_per_frame_data_mutex);
		m_per_frame_data_map.clear();
	}

	// get an immutable reference to the per frame data map
	const std::unordered_map<const char*, f32>& get_per_frame_data() const { return m_per_frame_data_map; }
private:
	// mutex for a scoped lock to ensure synchronization between render and main thread
	std::mutex m_per_frame_data_mutex{};
	// map of per frame data 
	std::unordered_map<const char*, f32> m_per_frame_data_map;
};

	
} // end namespace Kablunk
#endif
