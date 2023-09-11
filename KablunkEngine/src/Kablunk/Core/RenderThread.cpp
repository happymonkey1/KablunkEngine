#include "kablunkpch.h"
#include "Kablunk/Core/RenderThread.h"

#include "Kablunk/Renderer/RenderCommand.h"

#ifdef KB_PLATFORM_WINDOWS
#	include <Windows.h>
#else
#	error "trying to include windows platform code on non-windows platform?"
#endif


namespace kb
{ // start namespace Kablunk

struct render_thread_data_t
{
	CRITICAL_SECTION m_critical_section;
	CONDITION_VARIABLE m_condition_variable;

	thread_state_t m_state = thread_state_t::idle;
};

render_thread::render_thread(threading_policy_t p_engine_threading_policy)
	: m_thread{ "Rendering thread" }, m_threading_policy{ p_engine_threading_policy }
{
	m_data = new render_thread_data_t{};

	if (m_threading_policy == threading_policy_t::multi_threaded)
	{
		InitializeCriticalSection(&m_data->m_critical_section);
		InitializeConditionVariable(&m_data->m_condition_variable);
	}
}

render_thread::~render_thread()
{
	if (m_threading_policy != threading_policy_t::multi_threaded)
		return;

	DeleteCriticalSection(&m_data->m_critical_section);
}

void render_thread::run()
{
    KB_PROFILE_FUNC();

	m_running = true;
	if (m_threading_policy != threading_policy_t::multi_threaded)
		return;

	m_thread.dispatch(render::details::render_thread_func, this);
}

void render_thread::terminate()
{
    KB_PROFILE_FUNC();

	m_running = false;
	// make sure we finish processing any submitted functions before termination
	pump(); 

	if (m_threading_policy == threading_policy_t::multi_threaded)
		m_thread.join();
}

void render_thread::wait(thread_state_t p_wait_state)
{
    KB_PROFILE_FUNC();

	if (m_threading_policy == threading_policy_t::single_threaded)
		return;

	EnterCriticalSection(&m_data->m_critical_section);
	while (m_data->m_state != p_wait_state)
	{
		// yield and release critical section as an atomic operation
		SleepConditionVariableCS(&m_data->m_condition_variable, &m_data->m_critical_section, INFINITE);
	}
	LeaveCriticalSection(&m_data->m_critical_section);
}

void render_thread::wait_and_set(thread_state_t p_wait_state, thread_state_t p_post_wait_state)
{
    KB_PROFILE_FUNC();

	if (m_threading_policy == threading_policy_t::single_threaded)
		return;

	EnterCriticalSection(&m_data->m_critical_section);
	while (m_data->m_state != p_wait_state)
	{
		// yield and release critical section as an atomic operation
		SleepConditionVariableCS(&m_data->m_condition_variable, &m_data->m_critical_section, INFINITE);
	}
	m_data->m_state = p_post_wait_state;
	WakeAllConditionVariable(&m_data->m_condition_variable);
	LeaveCriticalSection(&m_data->m_critical_section);
}

void render_thread::set(thread_state_t p_state)
{
    KB_PROFILE_FUNC();

	if (m_threading_policy == threading_policy_t::single_threaded)
		return;

	EnterCriticalSection(&m_data->m_critical_section);
	m_data->m_state = p_state;
	WakeAllConditionVariable(&m_data->m_condition_variable);
	LeaveCriticalSection(&m_data->m_critical_section);
}

void render_thread::next_frame()
{
    KB_PROFILE_FUNC();

	m_app_thread_frame++;
	render::swap_queues();
}

void render_thread::block_until_rendering_complete()
{
    KB_PROFILE_FUNC();

	if (m_threading_policy == threading_policy_t::single_threaded)
		return;

	wait(thread_state_t::idle);
}

void render_thread::kick()
{
    KB_PROFILE_FUNC();

	if (m_threading_policy == threading_policy_t::multi_threaded)
		set(thread_state_t::kick);
	else
		render::wait_and_render(this);
}

void render_thread::pump()
{
    KB_PROFILE_FUNC();

	next_frame();
	kick();
	block_until_rendering_complete();
}

render_thread& render_thread::operator=(render_thread&& other) noexcept
{
	KB_CORE_ASSERT(!m_running, "cannot assign render thread while it is currently running!");
	KB_CORE_ASSERT(!other.m_running, "cannot assign render thread by move while other thread is running!");

	// cleanup old data
	if (m_data)
	{
		if (m_threading_policy == threading_policy_t::multi_threaded)
			DeleteCriticalSection(&m_data->m_critical_section);
		delete m_data;
	}
	
	m_data = other.m_data;
	m_running = other.m_data;
	m_thread = std::move(other.m_thread);
	m_threading_policy = other.m_threading_policy;

	return *this;
}

} // end namespace Kablunk
