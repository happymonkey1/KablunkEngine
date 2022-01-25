#ifndef EDITOR_EDITORLAYER_H
#define EDITOR_EDITORLAYER_H

#include <Kablunk.h>
#include "Kablunk/Renderer/EditorCamera.h"

#include "Panels/SceneHierarchyPanel.h"
#include "Panels/ContentBrowserPanel.h"
#include "Panels/ProjectPropertiesPanel.h"



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

		void CreateProject(std::filesystem::path project_path);
		void OpenProject();
		void OpenProject(const std::string& filepath);
		void SaveProject();
		void CloseProject(bool unload = true);
		void ReplaceToken(const char* token, std::string& data, const std::string& new_token);
		void UpdateProjectEngineFiles();

		void NewScene();
		void SaveScene();
		void SaveSceneAs();
		void SerializeScene(IntrusiveRef<Scene> scene, const std::filesystem::path& path);
		void OpenScene();
		void OpenScene(const std::filesystem::path& path);

		void ViewportClickSelectEntity();

		void OnOverlayRender();

		void UI_DrawMenuBar();
		void UI_Toolbar();
		void UI_KablunkInstallPopup();

		void OnScenePlay();
		void OnSceneStop();

		std::pair<glm::vec3, glm::vec3> RayCast(const EditorCamera& camera, float mx, float my);
	private:

		IntrusiveRef<Scene> m_active_scene;
		IntrusiveRef<Scene> m_editor_scene;
		IntrusiveRef<Scene> m_runtime_scene;

		IntrusiveRef<SceneRenderer> m_viewport_renderer;
		std::filesystem::path m_editor_scene_path;

		EditorCamera m_editor_camera;

		int m_gizmo_type = -1;

		bool m_primary_camera_selected{ true };
		bool m_application_paused{ true };
		bool m_viewport_focused{ false }, m_viewport_hovered{ false };
		bool m_show_debug_panel{ true };
		bool m_show_physics_colliders{ false };

		bool m_show_create_new_project_popup{ false };
		bool m_show_project_properties_panel{ false };
		bool m_show_replace_kablunk_install_popup{ false };

		glm::vec2 m_viewport_size{ 0.0f };
		glm::vec2 m_viewport_bounds[2];

		Entity m_selected_entity;

		// Panels
		SceneHierarchyPanel m_scene_hierarchy_panel;
		ContentBrowserPanel m_content_browser_panel;
		ProjectPropertiesPanel m_project_properties_panel;


		// Resources
		IntrusiveRef<Texture2D> m_icon_play, m_icon_stop;

		enum class SceneState
		{
			Edit = 0, Play = 1
		};
		SceneState m_scene_state{ SceneState::Edit };

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
