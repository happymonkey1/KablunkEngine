#include "kablunkpch.h"

#include "Kablunk/Project/Project.h"

namespace Kablunk
{

	IntrusiveRef<Project> Project::s_active_project = nullptr;

	Project::Project()
	{

	}

	Project::~Project()
	{

	}

	void Project::SetActive(const IntrusiveRef<Project>& project)
	{
		s_active_project = project;
	}
}
