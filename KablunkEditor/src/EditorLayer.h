#ifndef EDITOR_EDITORLAYER_H
#define EDITOR_EDITORLAYER_H

#include <Kablunk.h>
#include "Panels/SceneHierarchyPanel.h"

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
		glm::vec2 m_square_pos{ 0.5f, 0.5f };
		glm::vec2 m_square_size{ 0.5f, 0.5f };
		float m_square_rotation{ 0.0 };
		glm::vec4 m_square_color{ 0.8f, 0.2f, 0.3f, 1.0f };

		OrthographicCameraController m_camera_controller;

		Ref<Framebuffer> m_frame_buffer;
		Ref<Texture2D> m_missing_texture;
		Ref<Texture2D> m_kablunk_logo;

		Ref<Texture2D> m_icon_play;

		Ref<Scene> m_active_scene;
		Kablunk::Entity m_square_entity;

		Kablunk::Entity m_primary_camera_entity;
		Kablunk::Entity m_secondary_camera_entity;
		bool m_primary_camera_selected{ true };

		bool m_application_paused{ true };

		bool m_viewport_focused{ false }, m_viewport_hovered{ false };

		glm::vec2 m_viewport_size{ 0.0f };

		SceneHierarchyPanel m_hierarchy_panel;

		struct ProfileResult
		{
			const char* name;
			float time;
		};

		struct ImGuiProfilerStats
		{
			float Counter{ 0.0f };
			float Counter_max{ 0.1f };

			float Delta_time{ 10.0f };
			float Fps{ 10.0f };
		};
		ImGuiProfilerStats m_imgui_profiler_stats;
	};
}
#endif
