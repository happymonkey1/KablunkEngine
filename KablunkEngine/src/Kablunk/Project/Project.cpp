#include "kablunkpch.h"

#include "Kablunk/Project/Project.h"

namespace Kablunk
{

	Project::Project()
	{

	}

	Project::~Project()
	{

	}

	void Project::SetActive(const Ref<Project>& project)
	{
		s_active_project = project;
	}
}
