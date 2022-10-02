#include "kablunkpch.h"

#include "Kablunk/Project/Project.h"
#include "Kablunk/Asset/AssetManager.h"

#include "Kablunk/Core/Singleton.h"

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
		if (s_active_project)
		{
			Singleton<asset::AssetManager>::get().shutdown();
		}

		s_active_project = project;
		if (s_active_project)
		{
			Singleton<asset::AssetManager>::get().init();
		}
	}
}
