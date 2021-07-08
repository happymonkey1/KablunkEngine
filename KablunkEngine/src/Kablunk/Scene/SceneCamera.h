#ifndef KABLUNK_SCENE_SCENE_CAMERA_H
#define KABLUNK_SCENE_SCENE_CAMERA_H

#include "Kablunk/Renderer/Camera.h"

namespace Kablunk
{
	class SceneCamera : public Camera
	{
	public:
		enum class ProjectionType
		{
			Perspective = 0,
			Orthographic = 1
		};
	public:
		SceneCamera();
		virtual ~SceneCamera() = default;

		ProjectionType GetProjectionType() const { return m_projection_type; }
		void SetProjectionType(ProjectionType new_projection_type);

		void SetAspectRatio(float aspect_ratio) { m_aspect_ratio = aspect_ratio; }
		float GetAspectRatio() const { return m_aspect_ratio; }

		void SetOrthographic(float size, float near_clip, float far_clip);
		void SetPerspective(float vertical_fov, float near_clip, float far_clip);
		void SetViewportSize(uint32_t width, uint32_t height);

		// Perspective

		float GetPerspectiveVerticalFOV() const { return m_perspective_fov; }
		void SetPerspectiveVerticalFOV(float vertical_fov) { m_perspective_fov = vertical_fov; RecalculateProjection(); }
		
		float GetPerspectiveNearClip() const  { return m_perspective_near; }
		void SetPerspectiveNearClip(float near_clip) { m_perspective_near = near_clip; RecalculateProjection(); }

		float GetPerspectiveFarClip() const { return m_perspective_far; }
		void SetPerspectiveFarClip(float far_clip) { m_perspective_far = far_clip; RecalculateProjection(); }

		// Orthographic

		float GetOrthographicSize() const { return m_orthographic_size; }
		void SetOrthographicSize(float size) { m_orthographic_size = size; RecalculateProjection(); }

		float GetOrthographicNearClip() const { return m_orthographic_near; }
		void SetOrthographicNearClip(float near_clip) { m_orthographic_near = near_clip; RecalculateProjection(); }

		float GetOrthographicFarClip() const { return m_orthographic_far; }
		void SetOrthographicFarClip(float far_clip) { m_orthographic_far = far_clip; RecalculateProjection(); }


	private:
		void RecalculateProjection();
	private:
		ProjectionType m_projection_type{ ProjectionType::Orthographic };
		
		float m_perspective_fov{ glm::radians(45.0f) };
		float m_perspective_near = 0.01f, m_perspective_far = 1000.0f;
		
		float m_orthographic_size { 10.0f };
		float m_orthographic_near{ -1.0f }, m_orthographic_far{ 1.0f };


		float m_aspect_ratio{ 1.788f };

	};
}
#endif
