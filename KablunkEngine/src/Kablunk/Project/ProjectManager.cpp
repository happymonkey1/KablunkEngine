#include "kablunkpch.h"
#include "Kablunk/Project/ProjectManager.h"

#include "Kablunk/Asset/AssetManager.h"

namespace kb
{
	void ProjectManager::init()
	{

	}

	void ProjectManager::shutdown()
	{
		if (m_active_project)
			Singleton<asset::AssetManager>::get().shutdown();

		m_active_project = nullptr;
	}

	void ProjectManager::set_active(const ref<Project>& project)
	{
		if (m_active_project)
		{
			Singleton<asset::AssetManager>::get().shutdown();
		}

		m_active_project = project;
		if (m_active_project)
		{
			Singleton<asset::AssetManager>::get().init();
		}
	}
}
