#pragma once
#ifndef KABLUNK_PLATFORM_VULKAN_RENDER_API_H
#define KABLUNK_PLATFORM_VULKAN_RENDER_API_H

#include "Kablunk/Core/RefCounting.h"
#include "Kablunk/Renderer/RendererAPI.h"

#include "Platform/Vulkan/VulkanRenderPass.h"
#include "Platform/Vulkan/VulkanMaterial.h"
#include "Kablunk/Renderer/MaterialAsset.h"

#include <vulkan/vulkan.h>

namespace kb
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

		virtual void DrawIndexed(const ref<VertexArray>& vertexArray, uint32_t indexCount = 0) override;

		virtual void ClearImage(ref<RenderCommandBuffer> commandBuffer, ref<Image2D> image) override;

		virtual void SetWireframeMode(bool draw_wireframe) override { m_draw_wireframe = draw_wireframe; }

		// ======
		// Meshes
		// ======


		virtual void RenderMesh(ref<RenderCommandBuffer> render_command_buffer, ref<Pipeline> pipeline, ref<UniformBufferSet> uniform_buffer_set, ref<StorageBufferSet> storage_buffer_set, ref<Mesh> mesh, uint32_t submesh_index, ref<MaterialTable> material_table, ref<VertexBuffer> transform_buffer, uint32_t transform_offset, uint32_t instance_count) override;

		virtual void RenderMeshWithMaterial(ref<RenderCommandBuffer> render_command_buffer, ref<Pipeline> pipeline, ref<UniformBufferSet> uniform_buffer_set, ref<StorageBufferSet> storage_buffer_set, ref<Mesh> mesh, uint32_t submesh_index, ref<Material> material, ref<VertexBuffer> transform_buffer, uint32_t transform_offset, uint32_t instance_count, Buffer additional_uniforms) override;
		
		// #TODO submeshes
		virtual void render_instanced_submesh(
			ref<RenderCommandBuffer> render_command_buffer,
			ref<Pipeline> pipeline,
			ref<UniformBufferSet> uniform_buffer_set,
			ref<StorageBufferSet> storage_buffer_set,
			ref<Mesh> mesh,
			uint32_t submesh_index,
			ref<MaterialTable> material_table,
			ref<VertexBuffer> transform_buffer,
			uint32_t transform_offset,
			uint32_t instance_count
		) override;
		// #TODO instanced rendering

		virtual void SubmitFullscreenQuad(ref<RenderCommandBuffer> render_command_buffer, ref<Pipeline> pipeline, ref<UniformBufferSet> uniform_buffer_set, ref<StorageBufferSet> storage_buffer_set, ref<Material> material);

		virtual void RenderQuad(ref<RenderCommandBuffer> render_command_buffer, ref<Pipeline> pipeline, ref<UniformBufferSet> uniform_buffer_set, ref<StorageBufferSet> storage_buffer_set, ref<Material> material, const glm::mat4& transform) override;

		virtual void RenderGeometry(ref<RenderCommandBuffer> render_command_buffer, ref<Pipeline> pipeline, ref<UniformBufferSet> uniform_buffer_set, ref<StorageBufferSet> storage_buffer_set, ref<Material> material, ref<VertexBuffer> vertex_buffer, ref<IndexBuffer> index_buffer, const glm::mat4& transform, uint32_t index_count = 0) override;

		virtual void SetLineWidth(ref<RenderCommandBuffer> render_command_buffer, float line_width) override;

		virtual void BeginRenderPass(ref<RenderCommandBuffer> render_command_buffer, const ref<RenderPass>& render_pass, bool explicit_clear = false) override;
		virtual void EndRenderPass(ref<RenderCommandBuffer> render_command_buffer) override;

		virtual void WaitAndRender() override;

		// Vulkan Only
		static const std::vector<std::vector<VkWriteDescriptorSet>>& RT_RetrieveOrCreateUniformBufferWriteDescriptors(ref<UniformBufferSet> uniform_buffer_set, ref<VulkanMaterial> material);
		static const std::vector<std::vector<VkWriteDescriptorSet>>& RT_RetrieveOrCreateStorageBufferWriteDescriptors(ref<StorageBufferSet> storage_buffer_set, ref<VulkanMaterial> material);
		static void RT_UpdateMaterialForRendering(ref<VulkanMaterial> vulkan_material, ref<UniformBufferSet> uniform_buffer_set, ref<StorageBufferSet> storage_buffer_set);
		static VkDescriptorSet RT_AllocateDescriptorSet(VkDescriptorSetAllocateInfo& alloc_info);

	private:
		bool m_draw_wireframe{ false };
	};
}

#endif
