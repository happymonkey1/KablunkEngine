#pragma once
#ifndef KABLUNK_RENDERER_RENDERER_2D
#define KABLUNK_RENDERER_RENDERER_2D

#include "Kablunk/Core/RefCounting.h"

#include "Kablunk/Renderer/Texture.h"
#include "Kablunk/Renderer/OrthographicCamera.h"
#include "Kablunk/Renderer/EditorCamera.h"
#include "Kablunk/Renderer/Camera.h"
#include "Kablunk/Scene/Entity.h"

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

	class Renderer2D : public RefCounted
	{
	public:
		// Public API
		struct Renderer2DStats
		{
			Renderer2DStats() : Draw_calls{ 0 }, Quad_count{ 0 } { }
			uint32_t Draw_calls{ 0 };
			uint32_t Quad_count{ 0 };
			uint32_t Circle_count{ 0 };

			uint32_t GetTotalVertexCount() { return Quad_count * 4; }
			uint32_t GetTotalIndexCount() { return Quad_count * 6; }
		};

		static void Init();
		static void Shutdown();

		static IntrusiveRef<Texture2D> GetWhiteTexture();

		static void BeginScene(const Camera& camera, const glm::mat4& transform);
		static void BeginScene(const EditorCamera& camera);
		static void BeginScene(const OrthographicCamera& camera);
		static void EndScene();
		static void Flush();

		// Entity
		static void DrawSprite(Entity entity);

		// Texture
		static void DrawQuad(const glm::vec2& position, const glm::vec2& size, const IntrusiveRef<Texture2D>& texture, float tilingFactor = 1.0f, const glm::vec4& tintColor = glm::vec4{ 1.0f });
		static void DrawQuad(const glm::vec3& position, const glm::vec2& size, const IntrusiveRef<Texture2D>& texture, float tilingFactor = 1.0f, const glm::vec4& tintColor = glm::vec4{ 1.0f });
		// #TODO figure out how to pass 64 bit integers to OpenGL so we can support int64_t instead of int32_t
		static void DrawQuad(const glm::mat4& transform, const IntrusiveRef<Texture2D>& texture, float tilingFactor = 1.0f, const glm::vec4& tintColor = glm::vec4{ 1.0f }, int32_t entity_id = -1);

		static void DrawCircle(const glm::mat4& transform, const glm::vec4& color, float radius = 0.5f, float thickness = 1.0f, float fade = 0.005f, int32_t entity_id = -1);
	
		static void ResetStats();
		static Renderer2DStats GetStats();

		// Begin Renderer2D
		virtual void Renderer2D_Init() = 0;
		virtual void Renderer2D_Shutdown() = 0;

		virtual void Renderer2D_BeginScene(const Camera& camera, const glm::mat4& transform) = 0;
		virtual void Renderer2D_BeginScene(const EditorCamera& camera) = 0;
		virtual void Renderer2D_BeginScene(const OrthographicCamera& camera) = 0;
		virtual void Renderer2D_EndScene() = 0;
		virtual void Renderer2D_Flush() = 0;

		// Texture
		virtual void Renderer2D_DrawQuad(const glm::vec2& position, const glm::vec2& size, const IntrusiveRef<Texture2D>& texture, float tilingFactor = 1.0f, const glm::vec4& tintColor = glm::vec4{ 1.0f }) = 0;
		virtual void Renderer2D_DrawQuad(const glm::vec3& position, const glm::vec2& size, const IntrusiveRef<Texture2D>& texture, float tilingFactor = 1.0f, const glm::vec4& tintColor = glm::vec4{ 1.0f }) = 0;
		
		virtual void Renderer2D_DrawQuad(const glm::mat4& transform, const IntrusiveRef<Texture2D>& texture, float tilingFactor = 1.0f, const glm::vec4& tintColor = glm::vec4{ 1.0f }, int32_t entity_id = -1) = 0;

		virtual void Renderer2D_DrawCircle(const glm::mat4& transform, const glm::vec4& color, float radius = 0.5f, float thickness = 1.0f, float fade = 0.005f, int32_t entity_id = -1) = 0;
	protected:

		struct GenericRenderer2DData
		{
			static const uint32_t Max_quads = 20'000;
			static const uint32_t Max_vertices = Max_quads * 4;
			static const uint32_t Max_indices = Max_quads * 6;
			static const uint32_t Max_texture_slots = 32;
			glm::vec4 Quad_vertex_positions[4] = {};

			IntrusiveRef <VertexBuffer> Quad_vertex_buffer;
			IntrusiveRef <IndexBuffer> Quad_index_buffer;

			IntrusiveRef <VertexBuffer> Circle_vertex_buffer;
			IntrusiveRef <IndexBuffer> Circle_index_buffer;

			IntrusiveRef<Shader> Quad_shader;
			IntrusiveRef<Shader> Circle_shader;

			IntrusiveRef <Texture2D> White_texture;

			QuadVertex* Quad_vertex_buffer_base_ptr = nullptr;
			QuadVertex* Quad_vertex_buffer_ptr = nullptr;
			uint32_t Quad_count = 0;
			uint32_t Quad_index_count = 0;

			CircleVertex* Circle_vertex_buffer_base_ptr = nullptr;
			CircleVertex* Circle_vertex_buffer_ptr = nullptr;
			uint32_t Circle_count = 0;
			uint32_t Circle_index_count = 0;

			uint32_t Texture_slot_index = 1; //0 = white texture

			// TODO: change to asset handle when implemented
			std::array<IntrusiveRef<Texture2D>, Max_texture_slots> Texture_slots;

			Renderer2D::Renderer2DStats Stats;

			struct CameraData
			{
				glm::mat4 ViewProjection;
			};

			CameraData camera_buffer;
			IntrusiveRef<UniformBuffer> camera_uniform_buffer;
		};

		static GenericRenderer2DData GetGenericRendererData() { return s_generic_renderer_data; }
		inline static GenericRenderer2DData s_generic_renderer_data;

	private:
		static void StartNewBatch();
		static void EndBatch();
	private:
		inline static Renderer2D* s_renderer;
	};
}

#endif
