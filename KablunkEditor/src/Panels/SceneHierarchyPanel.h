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
	private:
		void DrawEntityNode(Entity entity);
		void DrawComponents(Entity entity);
	private:
		Ref<Scene> m_context;
		Entity m_selection_context;
	};
}

#endif
