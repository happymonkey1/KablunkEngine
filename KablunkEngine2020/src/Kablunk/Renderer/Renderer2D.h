#ifndef KABLUNK_RENDERER_RENDERER2D
#define KABLUNK_RENDERER_RENDERER2D

#include "OrthographicCamera.h"

namespace Kablunk
{
	class Renderer2D
	{
	public:
		static void Init();
		static void Shutdown();

		static void BeginScene(const OrthographicCamera& camera);
		static void EndScene();
	};
}

#endif