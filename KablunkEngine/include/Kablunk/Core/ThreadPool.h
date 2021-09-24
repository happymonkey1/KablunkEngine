#ifndef KABLUNK_CORE_THREAD_POOL_H
#define KABLUNK_CORE_THREAD_POOL_H

#include "Kablunk/Core/Core.h"

#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>

namespace Kablunk::Threading
{
	

	class ThreadPool
	{
	public:
		using JobFunc = std::function<void()>;
	public:

		ThreadPool() = delete;
		// 256 max threads
		ThreadPool(uint8_t num_threads);
		~ThreadPool();

		void AddJob(JobFunc& job);
		void Shutdown();
	private:
		bool m_terminate{ false }, m_has_stopped{ false };
		uint8_t m_max_threads;
		std::vector<std::thread> m_pool;

		std::queue<JobFunc> m_job_queue;
		std::mutex m_job_queue_mutex;
		std::condition_variable m_wait_for_job;

		void ThreadLoop();
	};

	

}

#endif
