#ifndef KABLUNK_SCENE_COMPONENT_H
#define KABLUNK_SCENE_COMPONENT_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define GLM_ENABLE_EXPERIMENTAL // 
#include <glm/gtx/quaternion.hpp>


#include "Kablunk/Scene/ScriptableEntity.h"
#include "Kablunk/Renderer/Texture.h"
#include "Kablunk/Scene/SceneCamera.h"


namespace Kablunk
{
	struct TagComponent
	{
		std::string Tag;

		TagComponent() = default;
		TagComponent(const TagComponent&) = default;
		TagComponent(const std::string& tag) : Tag{ tag } { }

		operator std::string& ()				{ return Tag; }
		operator const std::string& () const	{ return Tag; }
	};

	/*class Transform
	{
	public:
		glm::vec3 Position	= glm::vec3{ 1.0f };
		glm::vec3 Rotation	= glm::vec3{ 0.0f };
		glm::vec3 Scale		= glm::vec3{ 1.0f };


		Transform() = default;
		Transform(const Transform&) = default;
		Transform(const glm::mat4& transform) : m_transform{ transform }
		{
			glm::quat quaternion;
			glm::decompose(m_transform, Scale, quaternion, Position, m_skew, m_perspective);
			quaternion = glm::conjugate(quaternion);

			auto x = quaternion.x, y = quaternion.y, z = quaternion.z, w = quaternion.w;
			Rotation = { glm::degrees(x * w), glm::degrees(y * w), glm::degrees(z * w) };
		}
		Transform(const glm::vec3& pos, const glm::vec3& scale, const glm::vec3& rot) : Position{ pos }, Rotation{ rot }, Scale{ scale }
		{
			m_transform = glm::translate(glm::mat4(1.0f), Position)
				* glm::scale(glm::mat4(1.0f), Scale)
				* glm::eulerAngleYXZ(Rotation.y, Rotation.x, Rotation.z);
		}

		operator glm::mat4&() { return CalculateTransform(); }
		operator const glm::mat4&() const { return CalculateTransform(); }
	private:

		glm::mat4& CalculateTransform() const
		{
			m_transform = glm::translate(glm::mat4(1.0f), Position)
				* glm::scale(glm::mat4(1.0f), Scale)
				* glm::toMat4(glm::quat(Rotation));

			return m_transform;
		}

		mutable glm::mat4 m_transform{ 1.0f };
		glm::vec3 m_skew = glm::vec3{ 1.0f };
		glm::vec4 m_perspective = glm::vec4{ 1.0f };
	};*/

	struct TransformComponent
	{
		glm::vec3 Translation	= glm::vec3{ 0.0f };
		glm::vec3 Rotation		= glm::vec3{ 0.0f };
		glm::vec3 Scale			= glm::vec3{ 1.0f };

		TransformComponent() = default;
		TransformComponent(const TransformComponent&) = default;
		TransformComponent(const glm::vec3& translation) : Translation{ translation } { }

		glm::mat4 GetTransform() const
		{
			glm::mat4 rotation = glm::toMat4(glm::quat(Rotation));

			return glm::translate(glm::mat4{ 1.0f }, Translation)
				* rotation
				* glm::scale(glm::mat4{ 1.0f }, Scale);
		}

		operator glm::mat4 () const { return GetTransform(); }
	};

	struct SpriteRendererComponent
	{
		Ref<Texture2D> Texture{ Texture2D::Create(1, 1) };
		glm::vec4 Color{ 1.0f };
		float Tiling_factor{ 1.0f };

		SpriteRendererComponent() = default;
		SpriteRendererComponent(const SpriteRendererComponent&) = default;
		SpriteRendererComponent(glm::vec4 color) 
			: Color{ color } { }
		SpriteRendererComponent(const Ref<Texture2D>& texture, glm::vec4 color, float tiling_factor = 1.0f) 
			: Texture{ texture }, Color{ color }, Tiling_factor{ tiling_factor } { }
	};

	struct CameraComponent
	{
		SceneCamera Camera;
		bool Primary{ true };
		bool Fixed_aspect_ratio = false;

		CameraComponent() = default;
		CameraComponent(const CameraComponent& projection) = default;
	};

	struct NativeScriptComponent : ScriptableEntity
	{
		ScriptableEntity* Instance{ nullptr };

		std::function<void()>				InstantiateFunction;
		std::function<void()>				DestroyFunction;

		std::function<void()>				OnCreateFunction;
		std::function<void(Timestep)>		OnUpdateFunction;
		std::function<void()>				OnDestroyFunction;

		template <typename T>
		void Bind()
		{
			InstantiateFunction	= [&]() { Instance = new T(); };
			DestroyFunction		= [&]() { delete static_cast<T*>(Instance); Instance = nullptr; };

			OnCreateFunction	= [&]()				{ ((T*)Instance)->OnCreate(); };
			OnUpdateFunction	= [&](Timestep ts)	{ ((T*)Instance)->OnUpdate(ts); };
			OnDestroyFunction	= [&]()				{ ((T*)Instance)->OnDestroy(); };
		}
		
		friend class Scene;
	};
}

#endif
