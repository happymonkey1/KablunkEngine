#ifndef EDITOR_PANELS_SCENE_HEIRARCHY_PANEL_H
#define EDITOR_PANELS_SCENE_HEIRARCHY_PANEL_H

#include <Kablunk.h>
#include "Kablunk/Scene/Scene.h"
#include "Kablunk/Scene/Components.h"
#include "Kablunk/Scene/Entity.h"

namespace Kablunk
{
	// #TODO think about moving into runtime (KablunkEngine) because scene hierarchy and entity browsing could be useful for debugging runtime 

	class SceneHierarchyPanel
	{
	public:
		SceneHierarchyPanel() = default;
		SceneHierarchyPanel(const IntrusiveRef<Scene>& context);

		void SetContext(const IntrusiveRef<Scene>& context);
		
		void OnImGuiRender();
		
		Entity GetSelectedEntity() const { return m_selection_context; }
		
		void SetSelectionContext(EntityHandle handle) { m_selection_context = Entity{ handle, m_context.get() }; }
		void SetSelectionContext(Entity entity) { m_selection_context = entity; }

		void ClearSelectionContext() { m_selection_context = {}; }
	private:
		void UI_DrawEntityNode(Entity entity, bool draw_child_node = false);
		void UI_DrawComponents(Entity entity);
		void UI_RenameScenePopup();
	private:
		IntrusiveRef<Scene> m_context;
		Entity m_selection_context;

		// #TODO move to properties panel class
		bool m_display_debug_properties{ true };

		bool m_rename_scene_popup_open{ false };
	};
}

#endif
