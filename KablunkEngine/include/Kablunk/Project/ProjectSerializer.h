#ifndef KABLUNK_PROJECT_PROJECT_SERIALIZER_H
#define KABLUNK_PROJECT_PROJECT_SERIALIZER_H

#include "Kablunk/Core/Core.h"
#include "Kablunk/Project/Project.h"

namespace Kablunk
{
	class ProjectSerializer
	{
	public:
		ProjectSerializer(const ref<Project>& project);

		void Serialize(const std::string& filepath);
		bool Deserialize(const std::string& filepath);

	private:
		ref<Project> m_project;
	};
}

#endif
