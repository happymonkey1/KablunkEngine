#pragma once
#ifndef KABLUNK_PLATFORM_VULKAN_RENDER_API_H
#define KABLUNK_PLATFORM_VULKAN_RENDER_API_H

#include "Kablunk/Core/RefCounting.h"
#include "Kablunk/Renderer/RendererAPI.h"

#include "Platform/Vulkan/VulkanRenderPass.h"
#include "Platform/Vulkan/VulkanMaterial.h"

#include <vulkan/vulkan.h>

namespace Kablunk
{
	class VulkanRendererAPI : public RendererAPI
	{
	public:
		VulkanRendererAPI() {}
		virtual ~VulkanRendererAPI();

		virtual void Init() override;
		virtual void Shutdown() override;

		virtual void BeginFrame() override;
		virtual void EndFrame() override;

		virtual void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) override;

		virtual void SetClearColor(const glm::vec4& color) override;
		virtual void Clear() override;

		virtual void DrawIndexed(const IntrusiveRef<VertexArray>& vertexArray, uint32_t indexCount = 0) override;

		virtual void ClearImage(IntrusiveRef<RenderCommandBuffer> commandBuffer, IntrusiveRef<Image2D> image) override;

		virtual void SetWireframeMode(bool draw_wireframe) override { m_draw_wireframe = draw_wireframe; }

		// ======
		// Meshes
		// ======
		// #TODO static meshes
		virtual void RenderMeshWithMaterial(IntrusiveRef<RenderCommandBuffer> render_command_buffer, IntrusiveRef<Pipeline> pipeline, IntrusiveRef<UniformBufferSet> uniform_buffer_set, IntrusiveRef<StorageBufferSet> storage_buffer_set, IntrusiveRef<Mesh> mesh, uint32_t submesh_index, IntrusiveRef<Material> material, IntrusiveRef<VertexBuffer> transform_buffer, uint32_t transform_offset, uint32_t instance_count, Buffer additional_uniforms) override;
		// #TODO submeshes
		// #TODO instanced rendering




		virtual void SubmitFullscreenQuad(IntrusiveRef<RenderCommandBuffer> render_command_buffer, IntrusiveRef<Pipeline> pipeline, IntrusiveRef<UniformBufferSet> uniform_buffer_set, IntrusiveRef<StorageBufferSet> storage_buffer_set, IntrusiveRef<Material> material);


		virtual void RenderQuad(IntrusiveRef<RenderCommandBuffer> render_command_buffer, IntrusiveRef<Pipeline> pipeline, IntrusiveRef<UniformBufferSet> uniform_buffer_set, IntrusiveRef<StorageBuffer> storage_buffer_set, IntrusiveRef<Material> material, const glm::mat4& transform) override;
		
		virtual void RenderGeometry(IntrusiveRef<RenderCommandBuffer> render_command_buffer, IntrusiveRef<Pipeline> pipeline, IntrusiveRef<UniformBufferSet> uniform_buffer_set, IntrusiveRef<StorageBufferSet> storage_buffer_set, IntrusiveRef<Material> material, IntrusiveRef<VertexBuffer> vertex_buffer, IntrusiveRef<IndexBuffer> index_buffer, const glm::mat4& transform, uint32_t index_count = 0) override;

		virtual void SetLineWidth(IntrusiveRef<RenderCommandBuffer> render_command_buffer, float line_width) override;

		virtual void BeginRenderPass(IntrusiveRef<RenderCommandBuffer> render_command_buffer, const IntrusiveRef<RenderPass>& render_pass, bool explicit_clear = false) override;
		virtual void EndRenderPass(IntrusiveRef<RenderCommandBuffer> render_command_buffer) override;

		virtual void WaitAndRender() override;

		// Vulkan Only
		static const std::vector<std::vector<VkWriteDescriptorSet>>& VulkanRendererAPI::RT_RetrieveOrCreateUniformBufferWriteDescriptors(IntrusiveRef<UniformBufferSet> uniform_buffer_set, IntrusiveRef<VulkanMaterial> material);
		static const std::vector<std::vector<VkWriteDescriptorSet>>& VulkanRendererAPI::RT_RetrieveOrCreateStorageBufferWriteDescriptors(IntrusiveRef<StorageBufferSet> storage_buffer_set, IntrusiveRef<VulkanMaterial> material);
		static void RT_UpdateMaterialForRendering(IntrusiveRef<VulkanMaterial> vulkan_material, IntrusiveRef<UniformBufferSet> uniform_buffer_set, IntrusiveRef<StorageBufferSet> storage_buffer_set);
		static VkDescriptorSet RT_AllocateDescriptorSet(VkDescriptorSetAllocateInfo& alloc_info);
	private:
		bool m_draw_wireframe{ false };
	};
}

#endif
