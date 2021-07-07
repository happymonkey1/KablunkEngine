#ifndef KABLUNK_SCENE_SCENE_SERIALIZER_H
#define KABLUNK_SCENE_SCENE_SERIALIZER_H

#include "Kablunk/Scene/Scene.h"

namespace Kablunk
{
	class SceneSerializer
	{
	public:
		SceneSerializer(const Ref<Scene>& scene);
	
		void Serialize(const std::string& filepath);
		void SerializeBinary(const std::string& filepath);

		bool Deserialize(const std::string& filepath);
		bool DeserializeBinary(const std::string& filepath);
	private:
		Ref<Scene> m_scene;
	};
}

#endif
