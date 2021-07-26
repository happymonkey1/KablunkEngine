#ifndef EDITOR_PANELS_SCENE_HEIRARCHY_PANEL_H
#define EDITOR_PANELS_SCENE_HEIRARCHY_PANEL_H

#include <Kablunk.h>
#include "Kablunk/Scene/Scene.h"
#include "Kablunk/Scene/Components.h"
#include "Kablunk/Scene/Entity.h"

namespace Kablunk
{
	class SceneHierarchyPanel
	{
	public:
		SceneHierarchyPanel() = default;
		SceneHierarchyPanel(const Ref<Scene>& context);

		void SetContext(const Ref<Scene>& context);
		
		void OnImGuiRender();
		
		Entity GetSelectedEntity() const { return m_selection_context; }
	private:
		void DrawEntityNode(Entity entity, bool draw_child_node = false);
		void DrawComponents(Entity entity);
	private:
		Ref<Scene> m_context;
		Entity m_selection_context;

		// #TODO move to properties panel class
		bool m_display_debug_properties{ true };
	};
}

#endif
