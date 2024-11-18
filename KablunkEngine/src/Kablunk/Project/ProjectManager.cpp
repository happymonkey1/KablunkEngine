#include "kablunkpch.h"
#include "Kablunk/Project/ProjectManager.h"

#include "Kablunk/Asset/AssetManager.h"
#include "Kablunk/Core/Application.h"

namespace kb
{
void ProjectManager::init()
{
}

void ProjectManager::shutdown()
{
	if (m_active_project)
    {
		Singleton<asset::AssetManager>::get().shutdown();

#if APP_OWNED_RENDERER_2D
        Application::Get().get_renderer_2d()->set_asset_manager(arc<asset::AssetManager>{});
#endif
    }

	m_active_project = nullptr;
}

void ProjectManager::set_active(const arc<Project>& project)
{
	if (m_active_project)
	{
		Singleton<asset::AssetManager>::get().shutdown();
	}

	m_active_project = project;
	if (m_active_project)
	{
		Singleton<asset::AssetManager>::get().init(m_active_project);

#if APP_OWNED_RENDERER_2D
        Application::Get().get_renderer_2d()->set_asset_manager(arc{ &Singleton<asset::AssetManager>::get() });
#endif
	}
}
}
