#ifndef KABLUNK_SCENE_SCENE_CAMERA_H
#define KABLUNK_SCENE_SCENE_CAMERA_H

#include "Kablunk/Renderer/Camera.h"

namespace Kablunk
{
	class SceneCamera : public Camera
	{
	public:
		SceneCamera();
		virtual ~SceneCamera() = default;

		void SetOrthographic(float size, float near_clip, float far_clip);
		void SetViewportSize(uint32_t width, uint32_t height);

		void SetOrthographicSize(float size) { m_orthographic_size = size; RecalculateProjection(); }
		float GetOrthographicSize() const { return m_orthographic_size; }
		
		void SetOrthographicNearFar(const glm::vec2& clip_vec) { SetOrthographicNearFar(clip_vec.x, clip_vec.y); }
		void SetOrthographicNearFar(float near_clip, float far_clip) { 
			m_orthographic_near = near_clip; 
			m_orthographic_far = far_clip; 
			RecalculateProjection(); 
		}
		glm::vec2 GetOrthographicNearFar() const { return { m_orthographic_near, m_orthographic_far }; }

	private:
		void RecalculateProjection();
	private:
		float m_orthographic_size { 10.0f };
		float m_orthographic_near{ -1.0f }, m_orthographic_far{ 1.0f };

		float m_aspect_ratio{ 1.788f };
	};
}
#endif
