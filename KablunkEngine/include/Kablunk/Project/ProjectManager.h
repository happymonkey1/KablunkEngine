#pragma once
#ifndef KABLUNK_PROJECT_PROJECT_MANAGER_H
#define KABLUNK_PROJECT_PROJECT_MANAGER_H

#include "Kablunk/Core/Core.h"
#include "Kablunk/Project/Project.h"

namespace kb
{

class ProjectManager
{
public:
    void init();
    void shutdown();

    arc<Project> get_active() { return m_active_project; }

    SINGLETON_GET_FUNC(ProjectManager);

    void set_active(const arc<Project>& project);

private:
    arc<Project> m_active_project{ nullptr };
};

}

#endif
