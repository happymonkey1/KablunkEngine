#ifndef KABLUNK_SCENE_COMPONENT_H
#define KABLUNK_SCENE_COMPONENT_H

#include <glm/glm.hpp>
#include "Kablunk/Scene/ScriptableEntity.h"
#include "Kablunk/Renderer/Texture.h"

namespace Kablunk
{
	struct TagComponent
	{
		std::string Tag;

		TagComponent() = default;
		TagComponent(const TagComponent&) = default;
		TagComponent(const std::string& tag) : Tag{ tag } { }

		operator std::string& () { return Tag; }
		operator const std::string& () const { return Tag; }
	};

	struct TransformComponent
	{
		glm::mat4 Transform{ 1.0f };

		TransformComponent() = default;
		TransformComponent(const TransformComponent&) = default;
		TransformComponent(glm::mat4 transform) : Transform{ transform } { }

		operator glm::mat4& () { return Transform; }
		operator const glm::mat4& () const { return Transform; }
	};

	struct SpriteRendererComponent
	{
		Ref<Texture2D> Texture{ Texture2D::Create(1, 1) };
		glm::vec4 Color{ 1.0f };
		float Tiling_factor{ 1.0f };

		SpriteRendererComponent() = default;
		SpriteRendererComponent(const SpriteRendererComponent&) = default;
		SpriteRendererComponent(const Ref<Texture2D>& texture, glm::vec4 color, float tiling_factor = 1.0f) 
			: Texture{ texture }, Color{ color }, Tiling_factor{ tiling_factor } { }
		SpriteRendererComponent(glm::vec4 color) : Color{ color } { }
	};

	struct NativeScriptComponent : ScriptableEntity
	{
		ScriptableEntity* Instance{ nullptr };

		std::function<void()>	InstantiateFunction;
		std::function<void()>	DestroyFunction;

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