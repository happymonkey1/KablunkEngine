#include "kablunkpch.h"
#include "Kablunk/Core/JobSystem.h"

#include "Kablunk/Core/Application.h"

namespace kb::Threading
{
	void JobSystem::AddJob(ThreadPool::JobFunc job)
	{
		Application::Get().GetThreadPool().AddJob(job);
	}
}
