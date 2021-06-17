#ifndef EDITOR_EDITORLAYER_H
#define EDITOR_EDITORLAYER_H

#include <Kablunk.h>

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

		bool m_viewport_focused{ false }, m_viewport_hovered{ false };

		glm::vec2 m_viewport_size{ 0.0f };

		struct ProfileResult
		{
			const char* name;
			float time;
		};

		float m_ImguiUpdateCounter = 0.0f;
		float m_ImguiUpdateCounterMax = .1f;
		float m_ImguiDeltaTime = 10.0f;
		float m_ImguiFPS = 10.0f;
	};
}
#endif
