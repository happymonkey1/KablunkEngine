#ifndef KABLUNK_SCENE_YAML_SPECIALIZED_SERIALIZATION_H
#define KABLUNK_SCENE_YAML_SPECIALIZED_SERIALIZATION_H

#include <glm/glm.hpp>
#include <yaml-cpp/yaml.h>

namespace Kablunk
{
	YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec3& member)
	{
		out << YAML::Flow << YAML::BeginSeq;
		out << member.x << member.y << member.z;
		out << YAML::EndSeq;

		return out;
	}

	YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec4& member)
	{
		out << YAML::Flow << YAML::BeginSeq;
		out << member.x << member.y << member.z << member.w;
		out << YAML::EndSeq;

		return out;
	}
}

namespace YAML
{
	template <>
	struct convert<glm::vec3>
	{
		static Node encode(const glm::vec3 rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
		}

		static bool decode(const Node& node, glm::vec3& rhs)
		{
			if (!node.IsSequence() || node.size() != 3)
				return false;

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			rhs.z = node[2].as<float>();
			return true;
		}
	};

	template <>
	struct convert<glm::vec4>
	{
		static Node encode(const glm::vec4 rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			node.push_back(rhs.w);
		}

		static bool decode(const Node& node, glm::vec4& rhs)
		{
			if (!node.IsSequence() || node.size() != 4)
				return false;

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			rhs.z = node[2].as<float>();
			rhs.w = node[3].as<float>();
			return true;
		}
	};
}


#endif
