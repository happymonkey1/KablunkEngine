#pragma once
#ifndef KABLUNK_PLATFORM_VULKAN_RENDERER_2D_H
#define KABLUNK_PLATFORM_VULKAN_RENDERER_2D_H

#include "Kablunk/Renderer/RendererAPI.h"
#include "Kablunk/Renderer/Renderer2D.h"

#include <Platform/Vulkan/VulkanRendererAPI.h>
#include <Platform/Vulkan/VulkanRenderCommandBuffer.h>
#include <Platform/Vulkan/VulkanPipeline.h>

#include "Kablunk/Renderer/SceneRenderer.h"

namespace Kablunk
{

	class VulkanRenderer2D : public Renderer2D
	{
	public:
		VulkanRenderer2D() = default;
		~VulkanRenderer2D() {};
		virtual void Renderer2D_Init() override;
		virtual void Renderer2D_Shutdown() override;

		virtual void Renderer2D_BeginScene(const Camera& camera, const glm::mat4& transform) override {};
		virtual void Renderer2D_BeginScene(const EditorCamera& camera) override {};
		virtual void Renderer2D_BeginScene(const OrthographicCamera& camera) override {};
		virtual void Renderer2D_EndScene() override {};
		virtual void Renderer2D_Flush() override;


		virtual void Renderer2D_SetSceneRenderer(IntrusiveRef<SceneRenderer> scene_renderer) override;

		// Texture
		virtual void Renderer2D_DrawQuad(const glm::vec2& position, const glm::vec2& size, const IntrusiveRef<Texture2D>& texture, float tilingFactor = 1.0f, const glm::vec4& tintColor = glm::vec4{ 1.0f }) override {};
		virtual void Renderer2D_DrawQuad(const glm::vec3& position, const glm::vec2& size, const IntrusiveRef<Texture2D>& texture, float tilingFactor = 1.0f, const glm::vec4& tintColor = glm::vec4{ 1.0f }) override {};

		virtual void Renderer2D_DrawQuad(const glm::mat4& transform, const IntrusiveRef<Texture2D>& texture, float tilingFactor = 1.0f, const glm::vec4& tintColor = glm::vec4{ 1.0f }, int32_t entity_id = -1) override {};

		virtual void Renderer2D_DrawCircle(const glm::mat4& transform, const glm::vec4& color, float radius = 0.5f, float thickness = 1.0f, float fade = 0.005f, int32_t entity_id = -1) override {};
	private:
		IntrusiveRef<RenderCommandBuffer> m_render_command_buffer;
		IntrusiveRef<SceneRenderer> m_scene_renderer;

		IntrusiveRef<Material> m_quad_material;
		IntrusiveRef<Material> m_circle_material;

		VulkanRendererAPI* m_vulkan_renderer;
	};
}

#endif
