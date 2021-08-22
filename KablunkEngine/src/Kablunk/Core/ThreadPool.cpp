#include "kablunkpch.h"
#include "Kablunk/Core/ThreadPool.h"

#include <mutex>


namespace Kablunk::Threading
{

	ThreadPool::ThreadPool(uint8_t num_threads)
		: m_max_threads{ num_threads }, m_pool{ std::vector<std::thread>{} }
	{
		m_pool.reserve(m_max_threads);
		for (size_t i = 0; i < m_max_threads; ++i)
		{
			m_pool.push_back(std::thread{ &ThreadPool::ThreadLoop, this });
		}
	}

	ThreadPool::~ThreadPool()
	{
		if (!m_has_stopped)
			Shutdown();
	}

	void ThreadPool::ThreadLoop()
	{
		while (true)
		{
			JobFunc job = nullptr;
			{
				std::unique_lock queue_lock{ m_job_queue_mutex };
				m_wait_for_job.wait(queue_lock, [this]() { return !m_job_queue.empty() || m_terminate; });
				
				if (!m_job_queue.empty())
				{
					job = m_job_queue.front();
					m_job_queue.pop();
				}

				if (m_terminate)
					break;
			}

			// Call the job function
			if (job)
			{
				try
				{
					job();
				}
				catch (const std::exception& ex) {
					KB_CORE_ERROR("Exception '{0}' occurred while executing job", ex.what());
				}
				catch (const std::string& ex) {
					KB_CORE_ERROR("Exception '{0}' occurred while executing job", ex.c_str());
				}
				catch (...) {
					KB_CORE_ERROR("Unknown exception occurred while executing job");
				}
			}
		}
	}

	void ThreadPool::AddJob(JobFunc& job)
	{
		{
			std::unique_lock queue_lock{ m_job_queue_mutex };
			m_job_queue.push(job);
		}

		m_wait_for_job.notify_one();
	}

	void ThreadPool::Shutdown()
	{
		{
			std::unique_lock queue_lock{ m_job_queue_mutex };
			m_terminate = true;
		}
		m_wait_for_job.notify_all();

		for (auto& thread : m_pool)
			thread.join();

		m_pool.clear();
		m_has_stopped = true;
	}
}
