#ifndef EDITOR_EDITORLAYER_H
#define EDITOR_EDITORLAYER_H

#include <Kablunk.h>
#include "Kablunk/Renderer/EditorCamera.h"

#include "Panels/SceneHierarchyPanel.h"
#include "Panels/ContentBrowserPanel.h"


namespace Kablunk
{
	class EditorLayer : public Layer
	{
	public:
		EditorLayer();
		virtual ~EditorLayer() = default;

		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnUpdate(Timestep ts) override;
		virtual void OnImGuiRender(Timestep ts) override;
		virtual void OnEvent(Event& e);
	private:
		bool OnKeyPressed(KeyPressedEvent& e);
		bool OnMouseButtonPressed(MouseButtonPressedEvent& e);

		bool CanPickFromViewport() const;

		void NewScene();
		void SaveSceneAs();
		void OpenScene();

		void ViewportClickSelectEntity();
	private:

		Ref<Framebuffer> m_frame_buffer;
		Ref<Scene> m_active_scene;

		EditorCamera m_editor_camera;

		int m_gizmo_type = -1;

		bool m_primary_camera_selected{ true };
		bool m_application_paused{ true };
		bool m_viewport_focused{ false }, m_viewport_hovered{ false };

		glm::vec2 m_viewport_size{ 0.0f };
		glm::vec2 m_viewport_bounds[2];

		Entity m_selected_entity;

		SceneHierarchyPanel m_scene_hierarchy_panel;
		ContentBrowserPanel m_content_browser_panel;

		struct ProfileResult
		{
			const char* name;
			float time;
		};

		struct ImGuiProfilerStats
		{
			float Counter{ 0.0f };
			float Counter_max{ 0.05f };

			float Delta_time{ 10.0f };
			float Fps{ 10.0f };
		};
		ImGuiProfilerStats m_imgui_profiler_stats;
	};
}
#endif
