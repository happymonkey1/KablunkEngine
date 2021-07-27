#ifndef KABLUNK_RENDERER_RENDERER2D
#define KABLUNK_RENDERER_RENDERER2D

#include "Kablunk/Renderer/Texture.h"
#include "Kablunk/Renderer/OrthographicCamera.h"
#include "Kablunk/Renderer/EditorCamera.h"
#include "Kablunk/Renderer/Camera.h"
#include "Kablunk/Scene/Entity.h"

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

		// Entity
		static void DrawSprite(Entity entity);

		// Texture
		static void DrawQuad(const glm::vec2& position, const glm::vec2& size, const Ref<Texture2D>& texture, float tilingFactor = 1.0f, const glm::vec4& tintColor = glm::vec4{ 1.0f });
		static void DrawQuad(const glm::vec3& position, const glm::vec2& size, const Ref<Texture2D>& texture, float tilingFactor = 1.0f, const glm::vec4& tintColor = glm::vec4{ 1.0f });
		// #TODO figure out how to pass 64 bit integers to OpenGL so we can support int64_t instead of int32_t
		static void DrawQuad(const glm::mat4& transform, const Ref<Texture2D>& texture, float tilingFactor = 1.0f, const glm::vec4& tintColor = glm::vec4{ 1.0f }, int32_t entity_id = -1);
	
		static void ResetStats();
		static Renderer2DStats GetStats();

		
	private:
		static void StartNewBatch();
		static void EndBatch();
	};
}

#endif
