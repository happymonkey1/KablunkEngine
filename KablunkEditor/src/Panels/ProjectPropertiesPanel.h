#ifndef KABLUNK_EDITOR_PANELS_PROJECT_PROPERTIES_PANEL
#define KABLUNK_EDITOR_PANELS_PROJECT_PROPERTIES_PANEL

#include <Kablunk.h> 
#include <Kablunk/Project/Project.h>

namespace kb
{

	class ProjectPropertiesPanel
	{
	public:
		ProjectPropertiesPanel(const ref<Project>& project);
		~ProjectPropertiesPanel() = default;

		void OnImGuiRender(bool& show);

	private:
		void UI_GeneralSettings();

	private:
		ref<Project> m_project;
		std::string m_default_scene;
	};

}

#endif
