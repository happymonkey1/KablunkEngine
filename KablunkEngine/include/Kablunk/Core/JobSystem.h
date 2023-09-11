#ifndef KABLUNK_CORE_JOB_SYSTEM_H
#define KABLUNK_CORE_JOB_SYSTEM_H

#include "Kablunk/Core/ThreadPool.h"

namespace kb::Threading
{
	class JobSystem
	{
	public:
		static void AddJob(ThreadPool::JobFunc job);
	};
}

#endif

