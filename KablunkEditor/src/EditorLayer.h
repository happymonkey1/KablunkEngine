#ifndef EDITOR_EDITORLAYER_H
#define EDITOR_EDITORLAYER_H

#include <Kablunk.h>
#include "Kablunk/Renderer/EditorCamera.h"

#include "Panels/SceneHierarchyPanel.h"
#include "Panels/ContentBrowserPanel.h"
#include "Panels/ProjectPropertiesPanel.h"
#include "Panels/AssetRegistryPanel.h"
#include "Panels/AssetEditorPanel.h"

namespace Kablunk
{
    // forward declaration
    class Renderer2D;

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

		void CreateProject(std::filesystem::path project_path, bool create_native_script_project);
		void OpenProject();
		void OpenProject(const std::string& filepath);
		void SaveProject();
		void CloseProject(bool unload = true);
		void ReplaceToken(const char* token, std::string& data, const std::string& new_token) const;
		void UpdateProjectEngineFiles();

		void NewScene();
		void SaveScene();
		void SaveSceneAs();
		void SerializeScene(ref<Scene> scene, const std::filesystem::path& path);
		void OpenScene();
		void OpenScene(const std::filesystem::path& path);

		void CreateNewNativeScript(const std::string& script_name) const;
		void LoadNativeScriptModule() const;

		void ViewportClickSelectEntity();

		void OnOverlayRender();

		void UI_DrawMenuBar();
		void UI_Toolbar();
		void UI_KablunkInstallPopup();

		void OnScenePlay();
		void OnSceneStop();

		std::pair<glm::vec3, glm::vec3> RayCast(const EditorCamera& camera, float mx, float my);
	private:

		ref<Scene> m_active_scene;
		ref<Scene> m_editor_scene;
		ref<Scene> m_runtime_scene;

		ref<SceneRenderer> m_viewport_renderer;
        ref<Renderer2D> m_renderer_2d;
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
		ref<AssetEditorPanel> m_asset_editor_panel;
		ContentBrowserPanel m_content_browser_panel;
		ProjectPropertiesPanel m_project_properties_panel;
		AssetRegistryPanel m_asset_registry_panel;
		// temporary "panel" for memory statistics
		bool m_show_memory_statistics_window = false;

		// Resources
		ref<Texture2D> m_icon_play, m_icon_stop, m_icon_pause;

		enum class SceneState
		{
			Edit = 0, Play = 1, Pause = 2
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
