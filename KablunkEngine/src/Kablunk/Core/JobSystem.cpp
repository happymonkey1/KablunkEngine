#include "kablunkpch.h"
#include "Kablunk/Core/JobSystem.h"

#include "Kablunk/Core/Application.h"

namespace Kablunk::Threading
{
	void JobSystem::AddJob(ThreadPool::JobFunc job)
	{
		Application::Get().GetThreadPool().AddJob(job);
	}
}
