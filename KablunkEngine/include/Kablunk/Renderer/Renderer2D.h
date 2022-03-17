#pragma once
#ifndef KABLUNK_RENDERER_RENDERER_2D
#define KABLUNK_RENDERER_RENDERER_2D

#include "Kablunk/Core/RefCounting.h"

#include "Kablunk/Renderer/Texture.h"
#include "Kablunk/Renderer/OrthographicCamera.h"
#include "Kablunk/Renderer/EditorCamera.h"
#include "Kablunk/Renderer/Camera.h"
#include "Kablunk/Scene/Entity.h"

#include "Kablunk/Renderer/SceneRenderer.h"

#include "Kablunk/Renderer/UniformBuffer.h"
#include "Kablunk/Renderer/Shader.h"
#include "Kablunk/Renderer/Buffer.h"

namespace Kablunk
{
	struct QuadVertex
	{
		glm::vec3 Position;
		glm::vec4 Color;
		glm::vec2 TexCoord;
		float TexIndex;
		float TilingFactor;
		// #TODO figure out how to pass 64 bit integers to OpenGL
		int32_t EntityID{ -1 };
	};

	struct CircleVertex
	{
		glm::vec3 WorldPosition;
		glm::vec3 LocalPosition;
		glm::vec4 Color;
		float Radius;
		float Thickness;
		float Fade;

		// #TODO figure out how to pass 64 bit integers to OpenGL
		int32_t EntityID{ -1 };
	};

	struct LineVertex
	{
		glm::vec3 Position;
		glm::vec4 Color;
	};

	struct Renderer2DSpecification
	{
		bool swap_chain_target = false;
	};

	class Renderer2D : public RefCounted
	{
	public:
		// Public API
		struct Renderer2DStats
		{
			uint32_t Draw_calls = 0;
			uint32_t Quad_count = 0;
			uint32_t Circle_count = 0;
			uint32_t batch_count = 0;

			uint32_t GetTotalVertexCount() { return Quad_count * 4; }
			uint32_t GetTotalIndexCount() { return Quad_count * 6; }
		};

		static void Init(Renderer2DSpecification spec = {});
		static void Shutdown();

		static IntrusiveRef<Texture2D> GetWhiteTexture();

		static void BeginScene(const Camera& camera, const glm::mat4& transform);
		static void BeginScene(const EditorCamera& camera);
		static void BeginScene(const OrthographicCamera& camera);
		static void EndScene();
		static void Flush();

		static void OnImGuiRender();

		static IntrusiveRef<RenderPass> GetTargetRenderPass();
		static void SetTargetRenderPass(IntrusiveRef<RenderPass> render_pass);
		static void OnRecreateSwapchain();

		// Entity
		static void DrawSprite(Entity entity);
		
		// Texture
		static void DrawQuad(const glm::vec2& position, const glm::vec2& size, const IntrusiveRef<Texture2D>& texture, float tilingFactor = 1.0f, const glm::vec4& tintColor = glm::vec4{ 1.0f });
		static void DrawQuad(const glm::vec3& position, const glm::vec2& size, const IntrusiveRef<Texture2D>& texture, float tilingFactor = 1.0f, const glm::vec4& tintColor = glm::vec4{ 1.0f });
		// #TODO figure out how to pass 64 bit integers to OpenGL so we can support int64_t instead of int32_t
		static void DrawQuad(const glm::mat4& transform, const IntrusiveRef<Texture2D>& texture, float tilingFactor = 1.0f, const glm::vec4& tintColor = glm::vec4{ 1.0f }, int32_t entity_id = -1);

		// Texture Atlas
		static void DrawQuadFromTextureAtlas(const glm::vec2& position, const glm::vec2& size, const IntrusiveRef<Texture2D>& texture, const glm::vec2* texture_atlas_offsets, float tiling_factor = 1.0f, const glm::vec4& tint_color = glm::vec4{ 1.0f });
		static void DrawQuadFromTextureAtlas(const glm::vec3& position, const glm::vec2& size, const IntrusiveRef<Texture2D>& texture, const glm::vec2* texture_atlas_offsets, float tiling_factor = 1.0f, const glm::vec4& tint_color = glm::vec4{ 1.0f });
		static void DrawQuadFromTextureAtlas(const glm::mat4& transform, const glm::vec2& size, const IntrusiveRef<Texture2D>& texture, const glm::vec2* texture_atlas_offsets, float tiling_factor = 1.0f, const glm::vec4& tint_color = glm::vec4{ 1.0f });

		static void DrawCircle(const glm::mat4& transform, const glm::vec4& color, float radius = 0.5f, float thickness = 1.0f, float fade = 0.005f, int32_t entity_id = -1);

		// Line
		static void DrawLine(const glm::vec3& p0, const glm::vec3& p1, const glm::vec4& color = glm::vec4{ 1.0f });

		// Rect
		static void DrawRect(const glm::vec2& position, const glm::vec2& size, float rotation = 0, const glm::vec4& color = glm::vec4{ 1.0f });
		static void DrawRect(const glm::vec3& position, const glm::vec2& size, float rotation = 0, const glm::vec4& color = glm::vec4{ 1.0f });

		static void ResetStats();
		static Renderer2DStats GetStats();
	private:
		static void StartNewBatch();
		static void EndBatch();
	};
}

#endif
