#ifndef KABLUNK_RENDERER_RENDERER2D
#define KABLUNK_RENDERER_RENDERER2D

#include "Kablunk/Renderer/Texture.h"
#include "OrthographicCamera.h"
#include "Kablunk/Renderer/EditorCamera.h"
#include "Camera.h"

namespace Kablunk
{
	class Renderer2D
	{
	public:

		struct Renderer2DStats
		{
			Renderer2DStats() : Draw_calls{ 0 }, Quad_count{ 0 } { }
			uint32_t Draw_calls{ 0 };
			uint32_t Quad_count{ 0 };

			uint32_t GetTotalVertexCount() { return Quad_count * 4; }
			uint32_t GetTotalIndexCount() { return Quad_count * 6; }
		};

		static void Init();
		static void Shutdown();

		static void BeginScene(const Camera& camera, const glm::mat4& transform);
		static void BeginScene(const EditorCamera& camera);
		static void BeginScene(const OrthographicCamera& camera);
		static void EndScene();
		static void Flush();

		/* 
		* #TODO Cleanup DrawQuad 'situation' 
		* Since all drawing now uses transforms, there might not be a need to differentiate between DrawQuad and DrawRotatedQuad
		* Somehow consolidate the functions 
		*/
		
		// Color
		static void DrawQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color);
		static void DrawQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color);
		static void DrawQuad(const glm::mat4& transform, const glm::vec4& color);

		// Texture
		static void DrawQuad(const glm::vec2& position, const glm::vec2& size, const Ref<Texture2D>& texture, float tilingFactor = 1.0f, const glm::vec4& tintColor = glm::vec4{ 1.0f });
		static void DrawQuad(const glm::vec3& position, const glm::vec2& size, const Ref<Texture2D>& texture, float tilingFactor = 1.0f, const glm::vec4& tintColor = glm::vec4{ 1.0f });
		static void DrawQuad(const glm::mat4& transform, const Ref<Texture2D>& texture, float tilingFactor = 1.0f, const glm::vec4& tintColor = glm::vec4{ 1.0f });

		// Color + rotation
		static void DrawRotatedQuad(const glm::vec2& position, const glm::vec2& size, float rotation, const glm::vec4& color);
		static void DrawRotatedQuad(const glm::vec3& position, const glm::vec2& size, float rotation, const glm::vec4& color);
		static void DrawRotatedQuad(const glm::mat4& transform, const glm::vec4& color);

		// Texture + rotation
		static void DrawRotatedQuad(const glm::vec2& position, const glm::vec2& size, float rotation, const Ref<Texture2D>& texture, float tilingFactor = 1.0f, const glm::vec4& tintColor = glm::vec4{ 1.0f });
		static void DrawRotatedQuad(const glm::vec3& position, const glm::vec2& size, float rotation, const Ref<Texture2D>& texture, float tilingFactor = 1.0f, const glm::vec4& tintColor = glm::vec4{ 1.0f });
		static void DrawRotatedQuad(const glm::mat4& transform, const Ref<Texture2D>& texture, float tilingFactor = 1.0f, const glm::vec4& tintColor = glm::vec4{ 1.0f });
	
		static void ResetStats();
		static Renderer2DStats GetStats();

		
	private:
		static void StartNewBatch();
		static void EndBatch();
	};
}

#endif
