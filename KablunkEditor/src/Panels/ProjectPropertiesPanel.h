#ifndef KABLUNK_EDITOR_PANELS_PROJECT_PROPERTIES_PANEL
#define KABLUNK_EDITOR_PANELS_PROJECT_PROPERTIES_PANEL

#include <Kablunk.h> 
#include <Kablunk/Project/Project.h>

namespace Kablunk
{

	class ProjectPropertiesPanel
	{
	public:
		ProjectPropertiesPanel(const Ref<Project>& project);
		~ProjectPropertiesPanel() = default;

		void OnImGuiRender(bool& show);

	private:
		void UI_GeneralSettings();

	private:
		Ref<Project> m_project;
		std::string m_default_scene;
	};

}

#endif
