#pragma once

#include "Kablunk/Core/Core.h"
#include "Kablunk/Core/Thread.h"

namespace Kablunk
{ // start namespace Kablunk

// forward declaration
struct render_thread_data_t;

// threading policy for the engine
enum class threading_policy_t : u8 
{
	none = 0,
	single_threaded,
	multi_threaded
};

// state of a given thread
enum class thread_state_t : u8
{
	idle = 0,
	busy,
	kick
};

class render_thread
{
public:
	render_thread(threading_policy_t p_engine_threading_policy);
	~render_thread();

	void run();
	void terminate();
	bool is_running() const { return m_running; }

	void wait(thread_state_t p_wait_state);
	void wait_and_set(thread_state_t p_wait_state, thread_state_t p_post_wait_state);
	void set(thread_state_t p_state);

	void next_frame();
	void block_until_rendering_complete();
	void kick();
	void pump();

	render_thread& operator=(render_thread&& other) noexcept;
private:
	// pointer to data used by the render thread
	render_thread_data_t* m_data = nullptr;
	// policy for threading of the core engine
	threading_policy_t m_threading_policy = threading_policy_t::none;
	// actual thread
	thread m_thread{};
	// flag for whether the thread is currently running
	bool m_running = false;
	// atomic frame counter
	std::atomic<size_t> m_app_thread_frame = 0;

	friend class Application;
};

} // end namespace Kablunk
