#include <Kablunk.h>
#include <Kablunk/Core/EntryPoint.h>

#include "EditorLayer.h"

namespace Kablunk
{
	class KablunkEditor : public Application
	{
	public:
		KablunkEditor(const ApplicationSpecification& specification)
			: Application{ specification }
		{
			PushLayer(new EditorLayer());
		}

		~KablunkEditor()
		{
		}
	};

	Kablunk::Application* Kablunk::CreateApplication()
	{
		ApplicationSpecification specification = {
			"Kablunk Editor",
			1600,
			900,
			false,
			false,
			"",
			true,
			true
		};
		return new KablunkEditor(specification);
	}
}
