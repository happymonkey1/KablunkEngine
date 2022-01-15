#pragma once
#ifndef KABLUNK_PLATFORM_VULKAN_RENDER_API_H
#define KABLUNK_PLATFORM_VULKAN_RENDER_API_H

#include "Kablunk/Core/RefCounting.h"
#include "Kablunk/Renderer/RendererAPI.h"

#include "Platform/Vulkan/VulkanRenderPass.h"

#include <vulkan/vulkan.h>

namespace Kablunk
{
	class VulkanRendererAPI : public RendererAPI
	{
	public:
		VulkanRendererAPI() {}
		virtual ~VulkanRendererAPI();

		virtual void Init() override;
		virtual void BeginFrame() override;
		virtual void EndFrame() override;

		virtual void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) override;

		virtual void SetClearColor(const glm::vec4& color) override;
		virtual void Clear() override;

		virtual void DrawIndexed(const IntrusiveRef<VertexArray>& vertexArray, uint32_t indexCount = 0) override;

		virtual void ClearImage(IntrusiveRef<RenderCommandBuffer> commandBuffer, IntrusiveRef<Image2D> image) override;

		virtual void SetWireframeMode(bool draw_wireframe) override { m_draw_wireframe = draw_wireframe; }

		virtual void SubmitFullscreenQuad(IntrusiveRef<RenderCommandBuffer> render_command_buffer, IntrusiveRef<Pipeline> pipeline, IntrusiveRef<UniformBufferSet> uniform_buffer_set, IntrusiveRef<StorageBufferSet> storage_buffer_set, IntrusiveRef<Material> material);

		virtual void WaitAndRender() override;

		// Vulkan Only
		static VkDescriptorSet RT_AllocateDescriptorSet(VkDescriptorSetAllocateInfo& alloc_info);
		static void BeginRenderPass(IntrusiveRef<RenderCommandBuffer> render_command_buffer, const IntrusiveRef<RenderPass>& render_pass, bool explicit_clear = false);
		static void EndRenderPass(IntrusiveRef<RenderCommandBuffer> render_command_buffer);
	private:
		bool m_draw_wireframe{ false };
	};
}

#endif
