#include <Kablunk.h>
#include <Kablunk/Core/EntryPoint.h>

#include "EditorLayer.h"

namespace Kablunk
{
	class KablunkEditor : public Application
	{
	public:
		KablunkEditor()
		{
			Application::Get().SetWindowTitle("Kablunk Editor");
			PushLayer(new EditorLayer());
		}

		~KablunkEditor()
		{
		}
	};

	Kablunk::Application* Kablunk::CreateApplication()
	{
		return new KablunkEditor();
	}
}