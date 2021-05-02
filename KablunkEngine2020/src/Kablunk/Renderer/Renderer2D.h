#ifndef KABLUNK_RENDERER_RENDERER2D
#define KABLUNK_RENDERER_RENDERER2D

#include "Kablunk/Renderer/Texture.h"
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

		// Color
		static void DrawQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color);
		static void DrawQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color);

		// Texture
		static void DrawQuad(const glm::vec2& position, const glm::vec2& size, const Ref<Texture2D>& texture);
		static void DrawQuad(const glm::vec3& position, const glm::vec2& size, const Ref<Texture2D>& texture);

		// Color + rotation
		static void DrawRotatedQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color, float rotation);
		static void DrawRotatedQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color, float rotation);
	};
}

#endif